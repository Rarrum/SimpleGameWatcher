# SimpleGameWatcher
Utilities for watching game states to automate control of simple streaming-friendly UI elements.

NOTE: This project is still in its extremely early stages of development.  Not much exists yet beyond a simple timer and auto-controls for a single scenario in a single game.

# Usage
Simply run it, select the game on the right, and click the buttons below it to bring up the supported UI for that game.

# Contrbibuting additional game-specific controls
TODO (The initial build only supports Lufia 2's Ancient Cave, run in various snes emulators)

# Known issues
- Need to improve memory scanning logic:
- * Currently we run into false positives in some cases (such as if an emulator has saved/loaded save states).  If you run into this, try restarting the emulator.
- * Sometimes we just can't find snes ram and the UI appears frozen - try entering the cave then resetting while it's running.
- On linux, scanning doesn't work until the user runs "sudo setcap cap_sys_ptrace=eip EasyAutoTracker" on the executable - need to automate setting this somehow.
- On windows, minimizing the main control window also minimizes all other windows.
- Need to revisit Qt's UI element allocation patterns - they currently seem to be leaking when opening and closing windows.
- A GitHub release for linux isn't working yet (need to figure out how to get GitHub to use a newer compiler), so you'll need to build that yourself.
- High CPU usage and possible UI lag if an emulator is running and we're actively searching for a game whose memory patterns have not been found yet in the emulator.  This might result in the timer not immediately starting on the first run - soft reboot the game and it should be fine after.
