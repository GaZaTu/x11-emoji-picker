# x11-emoji-picker

A dialog / emoji picker inspired by the Windows 10 emoji picker written for Linux systems that use XServer.

Works for my use case most of the time. 😅

*Disclaimer: Donk code ahead FDM*

## Motivation 🤔

I switched from Windows 10 to Linux at work and missed filling my emails with emojis. ~~(the KDE version we use doesn't have the builtin emoji picker yet)~~ (turns out that the KDE emoji picker only copies emojis to clipboard anyway so yea fuck that)

## Screenshots 😮

![dialog with search result](doc/search.png)
*dialog with search result*

![dialog with full emoji list](doc/list.png)
*dialog with full emoji list*

![dialog with github aliases](doc/aliases.png)
*dialog with github aliases*

## Installation 😉

Just build it yourself loooool 4head

## Building 🤓

This is a CMake project so you need `cmake`, `make` and some c++ compiler like `gcc`.

### Dependencies

- Qt5 (`apt install qtbase5-dev` ?)
- ICU (`apt install libicu-dev`)
- xdo (`apt install libxdo-dev`)

### CMake

- `mkdir -p build`
- `cd build`
- `cmake -DCMAKE_BUILD_TYPE=Release ..`
- `make`

### Codegen

I committed the generated code so you won't have to run any scripts.

- script [.scripts/create-emoji-map.js](.scripts/create-emoji-map.js) to create [src/emojis.hpp](src/emojis.hpp) and [src/emojis.cpp](src/emojis.cpp)
- script [.scripts/create-emoji-aliases-ini.js](.scripts/create-emoji-aliases-ini.js) to create [src/aliases/github-emojis.ini](src/aliases/github-emojis.ini)

## Usage 🧐

First you should add the emoji-picker executable as a keybind (however you do that in your window manager); I use `meta+.` just like the windows keybind.

The emoji picker will always write to the window that was active **before** starting the application. You have to restart it if you want to write to a different window.

After the emoji picker opened you can just begin typing to search for an emoji. Emojis that start with the text you entered will be shown below the text input. You can navigate these by using the **arrow keys**.

When you've selected your emoji you can press the **return key** to write it.

### Keybinds

- `Arrow keys` => navigate through shown emojis
- `Return` => write emoji to previously active window
- `Shift+Return` => write emoji to previously active window and close emoji picker
- `F1` => show recently used emojis (default view)
- `F2` => show list of all emojis (if you press this while searching, the view will jump to the currently selected emoji aswell)
- `F4` => close emoji picker and open settings file
- `Tab` => toggle between `F1` and `F2`

### Settings

The settings file should be located at `/home/<user>/.config/gazatu.xyz/emoji-picker.ini`.

- `[General] | activateWindowBeforeWritingByDefault` => `true` to activate windows by default before writing to them
- `[General] | aliasExactMatching` => `true` to only show aliased emojis when the search matches completely
- `[General] | customQssFilePath` => custom styling (colors and shit) for the emoji picker (refer to [src/main.qss](src/main.qss) for examples); useful if you don't like the dark theme
- `[General] | gendersDisabled` => `true` if you only want to see gender neutral emojis (jobs or family or w/e)
- `[General] | localeKey` => the emoji translation you want to use (en, de, fr, nl, da, it, pt, es, sv, pl, hr, cs, fi, el, hu) or empty if you want to use english with underscores
- `[General] | maxEmojiVersion` => set this to for example 12 to exclude emojis released after that or -1 to show all emojis
- `[General] | openAtMouseLocation` => `true` if you want to open the emoji picker dialog at the current mouse cursor
- `[General] | skinTonesDisabled` => `true` if you only want to see skin-tone neutral emojis (hands or jobs or family or w/e)
- `[General] | surroundAliasesWithColons` => `true` if you want emoji aliases to be in colons (:joy: for example)
- `[General] | swapEnterAndShiftEnter` => `true` if you want `Return` to close the window
- `[General] | useClipboardHackByDefault` => `true` to write emojis using `ctrl+v` by default (qt5 apps for example)
- `[General] | useSystemQtTheme` => `true` if you want to use the system qt theme (not recommended, usually only works with kde i think)
- `[General] | windowOpacity` => how seethrough the emoji picker is supposed to be (0 = invisible)
- `[activateWindowBeforeWritingExceptions]` => list of executables that should be an exception to the `[General] | activateWindowBeforeWritingByDefault` setting
- `[emojiAliasesIniFilePaths]` => list of custom emoji codes (refer to [src/aliases/github-emojis.ini](src/aliases/github-emojis.ini) for examples); points to the builtin list of github emojis by default ([https://github.com/ikatyang/emoji-cheat-sheet](https://github.com/ikatyang/emoji-cheat-sheet/blob/master/README.md)); also points to the builtin list of gitmoji emojis by default ([https://gitmoji.dev/](https://gitmoji.dev/))
- `[recentEmojis]` => list of recently used emojis (edited by the application itself)
- `[useClipboardHackExceptions]` => list of executables that should be an exception to the `[General] | useClipboardHackByDefault` setting

#### Notes

If emojis do not get written into your window: try adding that executable to the `[activateWindowBeforeWritingExceptions]` array or to the `[useClipboardHackExceptions]` array.

#### Defaults

```ini
[General]
activateWindowBeforeWritingByDefault=false
aliasExactMatching=false
customQssFilePath=
gendersDisabled=false
localeKey=
maxEmojiVersion=-1
openAtMouseLocation=false
skinTonesDisabled=false
surroundAliasesWithColons=true
swapEnterAndShiftEnter=false
useClipboardHackByDefault=false
useSystemQtTheme=false
windowOpacity=0.9

[activateWindowBeforeWritingExceptions]
1\processName=code
2\processName=code-oss
3\processName=chrome
4\processName=chromium
5\processName=chatterino
6\processName=kate
size=6

[emojiAliasesIniFilePaths]
1\path=:/aliases/github-emojis.ini
2\path=:/aliases/gitmoji-emojis.ini
size=2

[recentEmojis]
size=0

[useClipboardHackExceptions]
1\processName=chatterino
2\processName=kate
size=2
```

## Notes 📝

### Tested on:

- Manjaro KDE with KDE clipboard manager
- OpenSUSE KDE with KDE clipboard manager
- Debian i3 with parcellite
- Arch i3 with parcellite

## Contributors 🤗

- [zneix](https://github.com/zneix) 😊👌

## License 😈

Code licensed under the [MIT](https://opensource.org/licenses/MIT) license: [LICENSE](LICENSE)

Graphics licensed by [Twitter](https://github.com/twitter) under *CC-BY 4.0* at [https://github.com/twitter/twemoji](https://github.com/twitter/twemoji/blob/master/LICENSE-GRAPHICS)

Emoji list and translations licensed by [Unicode](https://github.com/unicode-org) at [https://github.com/unicode-org/cldr](https://github.com/unicode-org/cldr/blob/master/unicode-license.txt)
