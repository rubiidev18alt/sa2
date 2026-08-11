# Sonic Advance 2 Android port

This is an Android frontend for the existing portable SDL build. The game logic and rendering code remain in the main project; Android supplies the SDL application shell and touch UI.

## Build

Open the `android/` directory in Android Studio, let Gradle sync, and build `app`.

The first build automatically downloads SDL2 `release-2.30.3` into `ext/SDL2`. The project uses Android Gradle Plugin 8.7.3, Gradle 8.9, Android API 35, CMake 3.30.5, and NDK 26.3.11579264.

For the CI build, the host-side `tools/preproc` executable is built with the host C++ compiler and supplied to the Android CMake project. This preserves the repository's normal embedded-data preprocessing step without attempting to build the host tool for an Android ABI.

Command-line build from the repository root:

```sh
cd android
gradle assembleDebug
```

The APK is written to `android/app/build/outputs/apk/debug/app-debug.apk`.

## Controls

The overlay is designed like a GBA emulator:

- Left: D-pad
- Right: A/B
- Top: L/R
- Bottom: Select/Start
- Multi-touch is supported
- Physical keyboards/controllers continue to use the existing SDL input path

The touch overlay is drawn above the SDL surface and translates button presses into the same SDL keyboard events used by the existing desktop port, so gameplay input stays inside the existing platform abstraction.

## Architectures

The initial APK targets `arm64-v8a`, `armeabi-v7a`, and `x86_64`.

## ROM/data policy

No copyrighted ROM files are bundled by this port. Build and use the project according to the upstream repository's requirements and licenses.
