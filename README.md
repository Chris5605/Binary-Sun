# Binary Sun Distortion
_Project built with AI_

A Windows VST3 distortion effect built with JUCE and CMake.

## Build

Configure from the repository root, setting `JUCE_DIR` to your JUCE source folder:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR="C:\path\to\JUCE"
cmake --build build --config Release --target BinarySunDistortion_VST3
```

The plugin includes selectable Warm (Soft Clip), Hard Clip, Asymmetric Overdrive,
Fuzz, and Tube Saturation modes, Drive (0–36 dB) and Output level controls, a Tone
low-pass filter, an adjustable input noise gate, and Low/Mid/High boost/cut EQ knobs.
The gate combines up to 18 dB of input attenuation with a smoothed post-distortion
stage keyed from the clean input, providing up to 48 dB of additional attenuation
below the threshold. Automatic output compensation reduces the level by up to 9 dB
as Drive increases.
REAPER can scan the generated `.vst3` bundle directly from the build output directory.
