// BC(c)Grid - grid transition for Premiere Pro / After Effects, by BillyCreative.
// (c) 2026 BillyCreative. Windows rebuild of the macOS BCGrid.plugin v1.0; same
// match name, parameter IDs and render behaviour, so projects open on both platforms.

#include "AEConfig.h"
#ifdef AE_OS_WIN
  #ifndef NOMINMAX
    #define NOMINMAX  // keep std::min / std::max usable
  #endif
  #include <Windows.h>
#endif
#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "SPBasic.h"
#if defined(__has_include)
  #if __has_include("AE_PluginData.h")
    #include "AE_PluginData.h"  // PF_REGISTER_EFFECT_EXT2 (SDK 2022+)
  #endif
#endif

#include <cstdio>
#include <vector>

#include "BCGridCore.h"
#include "BCGridStrings.h"

#define BCG_MAJOR 1
#define BCG_MINOR 1
#define BCG_BUG 0
#define BCG_STAGE PF_Stage_DEVELOP
#define BCG_BUILD 1

// Parameter indices (order in the Effect Controls panel).
enum {
    P_INPUT = 0,
    P_LAYER,          // Segundo clip
    P_PROGRESS,       // Progreso
    P_COLUMNS,        // Columnas
    P_ROWS,           // Filas
    P_LINES,          // Lineas: Mostrar
    P_THICKNESS,      // Grosor de lineas (px)
    P_LINE_COLOR,     // Color de lineas
    P_PATTERN,        // Patron
    P_SEED,           // Semilla aleatoria
    P_FLASH,          // Destello: Activado
    P_FLASH_COLOR,    // Color del destello
    P_FLASH_AMOUNT,   // Intensidad del destello
    P_FLASH_FRAMES,   // Fotogramas del destello
    P_NUM_PARAMS
};

// Disk IDs: must never change (they are what saved projects store).
enum {
    ID_LAYER = 1,
    ID_PROGRESS = 2,
    ID_COLUMNS = 3,
    ID_ROWS = 4,
    ID_THICKNESS = 5,
    ID_PATTERN = 6,
    ID_SEED = 7,
    ID_FLASH = 8,
    ID_FLASH_COLOR = 9,
    ID_FLASH_AMOUNT = 10,
    ID_FLASH_FRAMES = 11,
    ID_LINES = 12,
    ID_LINE_COLOR = 13
};

// Premiere's "PF Transition Suite". Declared locally (same layout as the SDK) so the
// build does not depend on which SDK header ships it.
#define BCG_kPFTransitionSuite "PF Transition Suite"
typedef struct {
    PF_Err (*RegisterTransitionInputParam)(PF_ProgPtr effect_ref, PF_ParamIndex index);
    PF_Err (*RegisterTransitionStartParam)(PF_ProgPtr effect_ref, PF_ParamIndex index);
    PF_Err (*RegisterTransitionEndParam)(PF_ProgPtr effect_ref, PF_ParamIndex index);
} BCG_TransitionSuite;

static const A_long kPremiereApplId = 0x50724D72;  // 'PrMr'

// Acquires the transition suite (v2, then v1) when running inside Premiere.
// Returns the version acquired (0 = none).
static int AcquireTransitionSuite(PF_InData* in_data, const BCG_TransitionSuite** suiteP) {
    *suiteP = NULL;
    if (in_data->appl_id != kPremiereApplId || !in_data->pica_basicP) return 0;
    const void* p = NULL;
    if (in_data->pica_basicP->AcquireSuite(BCG_kPFTransitionSuite, 2, &p) == kSPNoError && p) {
        *suiteP = static_cast<const BCG_TransitionSuite*>(p);
        return 2;
    }
    p = NULL;
    if (in_data->pica_basicP->AcquireSuite(BCG_kPFTransitionSuite, 1, &p) == kSPNoError && p) {
        *suiteP = static_cast<const BCG_TransitionSuite*>(p);
        return 1;
    }
    return 0;
}

static void ReleaseTransitionSuite(PF_InData* in_data, int version) {
    if (version) in_data->pica_basicP->ReleaseSuite(BCG_kPFTransitionSuite, version);
}

static PF_Err About(PF_InData* in_data, PF_OutData* out_data) {
    std::snprintf(out_data->return_msg, sizeof(out_data->return_msg), "%s v%d.%d\r%s",
                  BCG_NAME_DISPLAY, BCG_MAJOR, BCG_MINOR, BCG_DESCRIPTION);
    return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_OutData* out_data) {
    out_data->my_version = PF_VERSION(BCG_MAJOR, BCG_MINOR, BCG_BUG, BCG_STAGE, BCG_BUILD);
    out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_PIX_INDEPENDENT |
                          PF_OutFlag_NON_PARAM_VARY;  // 0x02000404, matches the PiPL
    out_data->out_flags2 = 0;
    return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData* in_data, PF_OutData* out_data) {
    PF_Err err = PF_Err_NONE;
    PF_ParamDef def;

    // In Premiere, parameter 1 becomes the incoming clip of the transition.
    const BCG_TransitionSuite* ts = NULL;
    const int tsVersion = AcquireTransitionSuite(in_data, &ts);
    if (ts && ts->RegisterTransitionInputParam) {
        ts->RegisterTransitionInputParam(in_data->effect_ref, P_LAYER);
    }

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER(STR_SECOND_CLIP, PF_LayerDefault_NONE, ID_LAYER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(STR_PROGRESS, 0, 100, 0, 100, 0, PF_Precision_TENTHS,
                         PF_ValueDisplayFlag_PERCENT, 0, ID_PROGRESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER(STR_COLUMNS, 1, 100, 1, 32, 3, ID_COLUMNS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER(STR_ROWS, 1, 100, 1, 32, 3, ID_ROWS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX(STR_LINES, STR_SHOW, TRUE, 0, ID_LINES);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(STR_THICKNESS, 0, 200, 0, 40, 4, PF_Precision_TENTHS, 0, 0, ID_THICKNESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR(STR_LINE_COLOR, 0, 0, 0, ID_LINE_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP(STR_PATTERN, 6, bcgrid::kRandom, STR_PATTERN_ITEMS, ID_PATTERN);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER(STR_SEED, 0, 10000, 0, 100, 1, ID_SEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX(STR_FLASH, STR_ENABLED, TRUE, 0, ID_FLASH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR(STR_FLASH_COLOR, 255, 255, 255, ID_FLASH_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX(STR_FLASH_AMOUNT, 0, 100, 0, 100, 80, PF_Precision_TENTHS,
                         PF_ValueDisplayFlag_PERCENT, 0, ID_FLASH_AMOUNT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER(STR_FLASH_FRAMES, 1, 60, 1, 12, 4, ID_FLASH_FRAMES);

    if (!err) {
        ReleaseTransitionSuite(in_data, tsVersion);
        out_data->num_params = P_NUM_PARAMS;
    }
    return err;
}

static bcgrid::World ToWorld(const PF_LayerDef* ld) {
    bcgrid::World w;
    w.data = ld->data;
    w.rowbytes = ld->rowbytes;
    w.width = ld->width;
    w.height = ld->height;
    return w;
}

static PF_Err Render(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[],
                     PF_LayerDef* output) {
    PF_Err err = PF_Err_NONE, err2 = PF_Err_NONE;

    PF_ParamDef second;
    AEFX_CLR_STRUCT(second);
    ERR(PF_CHECKOUT_PARAM(in_data, P_LAYER, in_data->current_time, in_data->time_step,
                          in_data->time_scale, &second));
    if (err) return err;

    const int colsParam = params[P_COLUMNS]->u.sd.value;
    const int rowsParam = params[P_ROWS]->u.sd.value;
    double progress = params[P_PROGRESS]->u.fs_d.value / 100.0;

    // Inside Premiere the transition drives itself from its position in the cut.
    {
        const BCG_TransitionSuite* ts = NULL;
        const int v = AcquireTransitionSuite(in_data, &ts);
        if (v) {
            ReleaseTransitionSuite(in_data, v);
            const A_long total = in_data->total_time;
            A_long denom = total - in_data->time_step;
            double p = 0.0;
            if (denom > 0) {
                p = (double)in_data->current_time / (double)(A_u_long)denom;
            } else if (total >= 1) {
                p = (double)in_data->current_time / (double)(A_u_long)total;
            }
            if (p < 0.0) p = 0.0;
            if (p > 1.0) p = 1.0;
            progress = p;
        }
    }

    const double thicknessParam =
        params[P_LINES]->u.bd.value ? params[P_THICKNESS]->u.fs_d.value : 0.0;
    const PF_Pixel lineColor = params[P_LINE_COLOR]->u.cd.value;
    const PF_Pixel flashColor = params[P_FLASH_COLOR]->u.cd.value;
    const bool flashChecked = params[P_FLASH]->u.bd.value != 0;
    const double intensity = params[P_FLASH_AMOUNT]->u.fs_d.value / 100.0;
    const int pattern = params[P_PATTERN]->u.pd.value;
    const int seed = params[P_SEED]->u.sd.value;

    // Flash width: N frames out of the whole transition length.
    double frames = 1.0;
    if (in_data->time_step >= 1 && in_data->total_time >= 1) {
        frames = (double)(A_u_long)in_data->total_time / (double)(A_u_long)in_data->time_step;
    }
    if (frames < 1.0) frames = 1.0;
    double flashWidth = (double)params[P_FLASH_FRAMES]->u.sd.value / frames;
    if (flashWidth < 0.001) flashWidth = 0.001;

    const int cols = colsParam > 1 ? colsParam : 1;
    const int rows = rowsParam > 1 ? rowsParam : 1;

    try {
        const std::vector<int> rank = bcgrid::ComputeRanks(cols, rows, pattern, seed);

        bcgrid::Settings s;
        s.progress = progress;
        s.cols = cols;
        s.rows = rows;
        s.thickness = thicknessParam * (double)in_data->downsample_x.num /
                      (double)(A_u_long)in_data->downsample_x.den;
        s.flashOn = intensity > 0.0 && flashChecked;
        s.intensity = intensity;
        s.flashWidth = flashWidth;

        const PF_LayerDef* first = &params[P_INPUT]->u.ld;
        const PF_LayerDef* incoming = &second.u.ld;
        const bool haveSecond = incoming->data && incoming->width > 0 && incoming->height > 0;
        const bcgrid::World A = ToWorld(first);
        const bcgrid::World B = haveSecond ? ToWorld(incoming) : A;
        const bcgrid::World out = ToWorld(output);

        if (out.width >= 1 && out.height >= 1) {
            if (PF_WORLD_IS_DEEP(output)) {
                bcgrid::Pixel16 line;
                line.alpha = PF_MAX_CHAN16;
                line.red = bcgrid::To16(lineColor.red);
                line.green = bcgrid::To16(lineColor.green);
                line.blue = bcgrid::To16(lineColor.blue);
                bcgrid::Render<bcgrid::Pixel16>(A, B, out, s, rank, line,
                                                bcgrid::To16(flashColor.red),
                                                bcgrid::To16(flashColor.green),
                                                bcgrid::To16(flashColor.blue));
            } else {
                bcgrid::Pixel8 line;
                line.alpha = PF_MAX_CHAN8;
                line.red = lineColor.red;
                line.green = lineColor.green;
                line.blue = lineColor.blue;
                bcgrid::Render<bcgrid::Pixel8>(A, B, out, s, rank, line, flashColor.red,
                                               flashColor.green, flashColor.blue);
            }
        }
    } catch (...) {
        err = PF_Err_OUT_OF_MEMORY;
    }

    ERR2(PF_CHECKIN_PARAM(in_data, &second));
    return err ? err : err2;
}

extern "C" DllExport PF_Err PluginDataEntryFunction2(PF_PluginDataPtr inPtr,
                                                     PF_PluginDataCB2 inPluginDataCallBackPtr,
                                                     SPBasicSuite* inSPBasicSuitePtr,
                                                     const char* inHostName,
                                                     const char* inHostVersion) {
    PF_Err result = PF_Err_INVALID_CALLBACK;
    result = PF_REGISTER_EFFECT_EXT2(inPtr, inPluginDataCallBackPtr,
                                     BCG_NAME_UTF8,       // Name
                                     BCG_MATCH_NAME,      // Match Name
                                     BCG_CATEGORY,        // Category
                                     AE_RESERVED_INFO,    // Reserved Info
                                     "EffectMain",        // Entry point
                                     BCG_SUPPORT_URL);    // Support URL
    return result;
}

extern "C" DllExport PF_Err EffectMain(PF_Cmd cmd, PF_InData* in_data, PF_OutData* out_data,
                                       PF_ParamDef* params[], PF_LayerDef* output, void* extra) {
    PF_Err err = PF_Err_NONE;
    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:        err = About(in_data, out_data); break;
            case PF_Cmd_GLOBAL_SETUP: err = GlobalSetup(out_data); break;
            case PF_Cmd_PARAMS_SETUP: err = ParamsSetup(in_data, out_data); break;
            case PF_Cmd_RENDER:       err = Render(in_data, out_data, params, output); break;
            default: break;
        }
    } catch (PF_Err& thrown_err) {
        err = thrown_err;
    } catch (...) {
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    return err;
}
