# Multigrain
A simple stored sample granulator, written using the JUCE framework.

## Usage
### Parameters
- **Position**: Sets the playback position of the mGrains.
- **Duration**: Changes the grain duration by a factor ranging from 1 to 1000.
- **Speed**: At a setting of 100 %, the mGrains will play back at the original speed.
- **Num Grains**: Determines how many mGrains will be active at a time. If set to 2, the second grain will play at an offset of 180° compared to the first grain.
- **Position Random**: When set to 100%, mGrains are played back at a random position across the sample.

## Build
Add your JUCE repository (`develop` branch) to the root of this repository or use a symbolic link.
Use your favorite CMake tool to build the project. Or use an IDE that supports CMake (vscode has a great CMake plugin).


_currently only works in Standalone build target_
