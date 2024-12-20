# SimpleGameWatcher
Utilities for watching game states to automate control of simple streaming-friendly UI elements.

NOTE: This project is still in its extremely early stages of development.  Not much exists yet beyond simple timer and auto-controls for a single game mode.

# Usage
- Run the timer app.
- Select the game of choice and click "Enable Auto For Game".  It will begin scanning.
- Run the game in question.  Within a few seconds, it should say "Ready" next to the auto button.  Some games may only be recognized when they reach certain execution points, such as their title screen or main menu.
- Click any of the buttons to create timer windows for that game.  The timer should automatically start and progress in reaction to the game state changing.
- You can right click any of the timer windows to change their colors, and drag their borders to resize them.

# Game Support
The current build only supports Lufia 2's Ancient Cave (Gift Mode), run in various snes emulators.  The code and UI is laid it to support multiple games in the future.

# Known issues
- Memory scanning sometimes runs into false positives (using an emulator's cheat memory search or save states might trigger this).  If you run into this, try restarting the emulator.
- On linux, scanning doesn't work until the user runs "sudo setcap cap_sys_ptrace=eip EasyAutoTracker" on the executable - need to automate setting this somehow.
- On linux, Qt5 libraries need to be installed on the system before the released build will run.
- High CPU usage and UI lag if an emulator is running and we're actively searching for a game whose memory patterns have not been found yet in the emulator.
- Save/Restore layout currently only suports game-auto-controlled windows (manual controls aren't hooked up to it yet).
- Lufia 2 RAM pattern detection on the title screen doesn't currently work right unless gift mode is unlocked.
