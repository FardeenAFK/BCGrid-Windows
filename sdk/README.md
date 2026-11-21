# After Effects SDK

The build needs Adobe's **After Effects SDK for Windows** (free from
<https://developer.adobe.com/after-effects/>, behind an Adobe sign-in). Adobe does not allow it to be
redistributed, so it is not in this repository.

**Local build:** unzip the SDK anywhere inside this `sdk/` folder. The build finds `AE_Effect.h` and
uses the `Examples/Resources/PiPLtool.exe` next to it. Nothing in here except this file is committed.

**GitHub Actions:** the workflow gets the SDK from a private repository through the read-only
deploy key in the `SDK_DEPLOY_KEY` secret, or from a private link to the SDK zip in the `AE_SDK_URL`
secret. Forks need to set up one of the two themselves.
