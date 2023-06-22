# SimpleGameWatcher
Utilities for watching game states to automate control of simple streaming-friendly UI elements.

NOTE: This project is still in its extremely early stages of development.  Not much exists yet beyond simple timer and auto-controls for a single game mode.

# Usage
- Run the timer app.
- Select the game of choice and click "Enable Auto For Game".  It will begin scanning.
- Run the game in question.  Within a few seconds, it should say "Ready" next to the auto button.  Some games may only be recognized when they reach certain execution points, such as their title screen or main menu.
- Click any of the buttons to create timer windows for that game.  The timer should automatically start and progress in reaction to the game state changing.
- You can right click any of the timer windows to change their colors, and drag their borders to resize them.

# Contrbibuting additional game-specific controls
TODO (The initial build only supports Lufia 2's Ancient Cave, run in various snes emulators)

# Known issues
- Memory scanning sometimes runs into false positives (using an emulator's cheat memory search or save states might trigger this).  If you run into this, try restarting the emulator.
- On linux, scanning doesn't work until the user runs "sudo setcap cap_sys_ptrace=eip EasyAutoTracker" on the executable - need to automate setting this somehow.
- High CPU usage and UI lag if an emulator is running and we're actively searching for a game whose memory patterns have not been found yet in the emulator.
- Sometimes the mouse cursor gets stuck in resize mode when resizing windows.  Hovering back over the resize border usually fixes this.
