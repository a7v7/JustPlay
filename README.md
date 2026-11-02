# JustPlay

A tiny command-line WAV player for Windows. You give it a file and it plays the file. That's all it does.

## Why?

You have a `.wav` file and you'd like to hear it. That shouldn't be hard.

You double-click it, and for Some Reason™ VLC takes forever to start. It's still loading its plugins, building its UI and having a think about its life choices. By the time it's ready, you've forgotten what the file was for.

So you try Audacity. Don't get me started on Audacity. You wanted to *listen* to a file, and now you're staring at waveforms, a project window and a dialog box about something you didn't ask about.

Media players want to manage your library. Audio editors want to be your studio. You just want to hear the thing.

**JustPlay does the one thing.** It starts instantly, plays the sound on your default output device and exits.

## Usage

```
justplay <file.wav | directory>
```

| Command                    | What happens                                                  |
| -------------------------- | ------------------------------------------------------------- |
| `justplay`                 | Shows the version and a short help message                    |
| `justplay beep.wav`        | Plays `beep.wav` and exits when it's done                     |
| `justplay C:\sounds`       | Plays every `.wav` in the folder, in Explorer sort order      |

Explorer sort order means `track2.wav` plays before `track10.wav`, the same order you see in File Explorer.

There are no switches, no config files and no playlists.

### Double-clicking WAVs in Explorer

Use `justplayw.exe` as the default app for `.wav` files. It's the same player, but it doesn't open a console window, so clips play silently in the background. If something goes wrong (for example, a file that isn't really a WAV), it shows the error in a message box.

Use `justplay.exe` from the command line, where you want to see what's playing.

## Building

You'll need [MSYS2](https://www.msys2.org/) with the UCRT64 toolchain, CMake and Ninja:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

From an MSYS2 UCRT64 shell:

```bash
cmake --preset release
cmake --build --preset release
```

This produces `build/release/justplay.exe` and `build/release/justplayw.exe`. They're statically linked, so you can copy them anywhere on your `PATH` and they run without any MSYS2 DLLs. For a debug build, use the `debug` preset instead (output goes to `build/debug`).

## Limitations

- WAV files only. JustPlay uses the built-in Windows sound API, so formats Windows can't play natively (MP3, FLAC and so on) won't work.
- Subfolders aren't searched when you play a directory.
- Paths longer than 260 characters aren't supported.
- No pause, seek or volume control. Press Ctrl+C to quit.

## License

MIT. See [LICENSE](LICENSE).
