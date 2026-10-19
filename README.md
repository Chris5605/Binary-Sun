# Binary Sun Distortion
_Project built with AI_

A Windows VST3 distortion effect built with JUCE and CMake.

## Build

Configure from the repository root, setting `JUCE_DIR` to your JUCE source folder:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR="C:\path\to\JUCE"
cmake --build build --config Release --target BinarySunDistortion_VST3
```

The plugin includes two independently voiced parallel distortion engines, each with
Warm (Soft Clip), Hard Clip, Asymmetric Overdrive, Fuzz, and Tube Saturation modes.
Each engine has its own Drive control; shared Blend and Coupling controls mix the
engines and let low-frequency energy in Engine A dynamically push Engine B's
high-band Drive by up to 12 dB. A Linkwitz-Riley crossover routes low frequencies
to Engine A and high frequencies to Engine B; the Crossover control sets its split
point. Blend balances the two bands with equal-power gains, while Coupling also
applies matching level compensation to help keep the output controlled. The plugin
also has Output, Tone low-pass, adjustable input noise gate, and Low/Mid/High
boost/cut EQ controls.
The gate combines up to 18 dB of input attenuation with a smoothed post-distortion
stage keyed from the clean input, providing up to 48 dB of additional attenuation
below the threshold. Automatic output compensation reduces the level by up to 9 dB
as Drive increases.
The interface displays animated electric-blue waveform rings for both engines; their
shape and spacing react to engine settings. Controls are arranged in a mirrored
layout for quick comparison between the low and high engines.
REAPER can scan the generated `.vst3` bundle directly from the build output directory.

## Project Reflection

This is the first project I have undertaken that truly took advantage of AI. It was 
unsettling at first, not being in the drivers seat, and it almost felt too easy. 
However, it allowed me to build out a fairly robust plugin in just a few hours, which
would've probably taken weeks without help. I would like to continue to develop these 
plugins, especially with a more hands-on approach so that I can learn the intricacies 
of VST, but for now I think this is a good introduction. If you want to use the plugin
yourself, go ahead! I will tweak it here and there as I use it for my own use, so stay
tuned!
