# x11-emoji-picker

A dialog / emoji picker inspired by the Windows 10 emoji picker written for Linux systems using Xorg.

Works for my use case most of the time. 😅 😂👌🔥💯

## Motivation 🤔

I switched from Windows 10 to Linux at work and missed filling my emails with emojis. (the KDE version we use doesn't have the builtin emoji picker yet)

## Screenshots 😮

![dialog with search result](doc/search.png)
*dialog with search result*

![dialog with full emoji list](doc/list.png)
*dialog with full emoji list*

![dialog with github aliases](doc/aliases.png)
*dialog with github aliases*

## Installation 😉

Install Qt5, ICU and xdotool using your package manager.
Example for Debian based systems: `apt install qt5-default libicu libxdo`.

Download the emoji-picker executable from [here](https://github.com/GaZaTu/x11-emoji-picker/releases), or build it yourself.

## Building 🤓

This is a CMake project so you need cmake and some c++ compiler.
I think it uses g++ by default but tbh i have no idea how cmake actually works.
Actually i got it to work with clang+msvc on windows aswell so clang should work.

### Required Libs

- Qt5 (`apt install qtbase5-dev` ?)
- ICU (`apt install libicu-dev`)
- xdo (`apt install libxdo-dev`)

### CMake

- `mkdir build`
- `cd build`
- `cmake ..`
- `make .`

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

### Settings

The settings file should be located at `/home/<user>/.config/gazatu.xyz/emoji-picker.ini`.

- `[General] | activateWindowBeforeWritingByDefault` => enable this to activate windows by default before writing to them
- `[General] | aliasExactMatching` => enable this to only show aliased emojis when the search matches completely
- `[General] | copyEmojiToClipboardAswellByDefault` => enable this to copy emojis into clipboard after trying to write them
- `[General] | customQssFilePath` => custom styling (colors and shit) for the emoji picker (refer to [src/main.qss](src/main.qss) for examples); useful if you don't like the dark theme
- `[General] | emojiAliasesIniFilePath` => list of custom emoji codes (refer to [src/aliases/github-emojis.ini](src/aliases/github-emojis.ini) for examples); points to the builtin list of github emojis by default ([https://github.com/ikatyang/emoji-cheat-sheet/blob/master/README.md](https://github.com/ikatyang/emoji-cheat-sheet/blob/master/README.md))
- `[General] | gendersDisabled` => `true` if you only want to see gender neutral emojis (jobs or family or w/e)
- `[General] | localeKey` => the emoji translation you want to use (en, de, fr, nl, da, it, pt, es, sv, pl, hr, cs, fi, el, hu) or empty if you want to use english with underscores
- `[General] | maxEmojiVersion` => set this to for example 12 to exclude emojis released after that or -1 to show all emojis
- `[General] | openAtMouseLocation` => enable this if you want to open the emoji picker dialog at the current mouse cursor
- `[General] | skinTonesDisabled` => `true` if you only want to see skin-tone neutral emojis (hands or jobs or family or w/e)
- `[General] | useSystemQtTheme` => enable this if you want to use the system qt theme (not recommended, usually only works with kde i think)
- `[activateWindowBeforeWritingExceptions]` => list of executables that should be an exception to the `[General] | activateWindowBeforeWritingByDefault` setting
- `[copyEmojiToClipboardAswellExceptions]` => list of executables that should be an exception to the `[General] | copyEmojiToClipboardAswellByDefault` setting
- `[recentEmojis]` => list of recently used emojis (edited by the application itself)

#### Defaults

```ini
[General]
activateWindowBeforeWritingByDefault=false
aliasExactMatching=false
copyEmojiToClipboardAswellByDefault=false
customQssFilePath=
emojiAliasesIniFilePath=:/aliases/github-emojis.ini
gendersDisabled=false
localeKey=
maxEmojiVersion=-1
openAtMouseLocation=false
skinTonesDisabled=false
useSystemQtTheme=false

[activateWindowBeforeWritingExceptions]
1\processName=code
2\processName=chromium
size=2

[copyEmojiToClipboardAswellExceptions]
1\processName=example
size=1

[recentEmojis]
size=0
```

## Credits 🤗

- [zneix](https://github.com/zneix) 😊👌

## License 😈

Code licensed under the [MIT](https://opensource.org/licenses/MIT) license: [LICENSE](LICENSE)

Graphics licensed by [Twitter](https://github.com/twitter) under *CC-BY 4.0* at [https://github.com/twitter/twemoji](https://github.com/twitter/twemoji)

Emoji list and translations licensed by [Unicode](https://github.com/unicode-org) at [https://github.com/unicode-org/cldr](https://github.com/unicode-org/cldr/blob/master/unicode-license.txt)
