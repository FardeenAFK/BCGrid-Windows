; BC©Grid - Windows installer (Inno Setup 6.3+)
; © 2026 BillyCreative
#define AppVer "1.0"

[Setup]
AppId={{6C1E7A52-3B7F-4C59-9E7B-0B8C2A6E4D11}
AppName=BC©Grid
AppVersion={#AppVer}
AppVerName=BC©Grid {#AppVer}
AppPublisher=BillyCreative
AppPublisherURL=https://billycreative.com
AppCopyright=© 2026 BillyCreative
DefaultDirName={commonpf64}\Adobe\Common\Plug-ins\7.0\MediaCore\BillyCreative
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=no
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=BC_Grid_Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=BC©Grid (BillyCreative)
VersionInfoVersion=1.0.0.1
VersionInfoCompany=BillyCreative
VersionInfoDescription=Instalador de BC©Grid para Premiere Pro

[Languages]
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"

[Messages]
es.WelcomeLabel2=Esto instalará la transición BC©Grid para Adobe Premiere Pro (y After Effects).%n%nCierra Premiere Pro antes de continuar.

[Files]
Source: "..\build\BCGrid.aex"; DestDir: "{app}"; Flags: ignoreversion

[Code]
function IsRunning(const ExeName: String): Boolean;
var
  ResultCode: Integer;
begin
  Result := False;
  if Exec(ExpandConstant('{cmd}'), '/C tasklist /FI "IMAGENAME eq ' + ExeName + '" | find /I "' + ExeName + '" >nul',
          '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    Result := (ResultCode = 0);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if CurPageID = wpReady then
    while IsRunning('Adobe Premiere Pro.exe') or IsRunning('AfterFX.exe') do
    begin
      if MsgBox('Premiere Pro o After Effects está abierto. Ciérralo y pulsa Aceptar.', mbInformation, MB_OKCANCEL) = IDCANCEL then
      begin
        Result := False;
        Exit;
      end;
    end;
end;
