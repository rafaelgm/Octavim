# Octavim

This plugin makes it possible to control Octave through Vim. It has two main features:

- It tries to understand cell blocks (`%%` in Matlab), allowing to navigate between them.
- It allows to run the selected text, the current cell or the entire file.

## Usage

Open a `.m` file on Vim and open Octave (personally I prefer to position their windows side by side).
To navigate between cell blocks (if any), press this while in normal mode:
- `ctrl+j`: jumps to the next cell
- `ctrl+k`: jumps to the previous cell
 
To run some code, press:
- `ctrl+return`: runs the current cell (works in normal and insert mode)
- `f9`: runs the selected text (in visual mode)
- `f5`: runs the entire file

`NOTE:` I strongly recommend to keep only the command window open on Octave, or else there is a chance your commands will be sent to somewhere else (like the Octave editor, if it's open).

### See it in action

![](https://cloud.githubusercontent.com/assets/6760593/11398926/028dd9a6-936b-11e5-8a8c-89f43eab0826.gif)

## How it works

Although the plugin code is written in Vim Script (obviously), it relies on a C library (source is included in the plugin). The plugin calls the library, passing the text to be run. The library then activates Octave window, using Windows API, and send the text simulating a Ctrl+C/Ctrl+V. It tries to save/restore the current text on the clipboard.

## Installation instructions (using [NeoBundle](https://github.com/Shougo/neobundle.vim))

Put this on your vimrc
```
NeoBundle 'rafaelgm/Octavim'
```

After the plugin is installed, it's time to build the helper library:
- Open Octave
- Set the current folder to `[Your home folder]\.vim\bundle\Octavim\tools`
- Run `build` 

It will call `gcc` (assuming Octave has it) and compile the DLL.

### Author's note
I've created this as a personal plugin to control Octave through Vim, but decided to share it's code. First I found [vim-matlab-behave](https://github.com/elmanuelito/vim-matlab-behave), but that only works on Linux. At first, I decided I would port it to Windows, but in the end I wrote Octavim from scratch, also as an excuse to learn Vim Script. :)

I didn't have much time to spend on this project, so the code is **far from perfect and optimal** (did my best given the time I had), but it works for me. Also, the code is targeted to work on Windows, using gVim 7.4 (32 bits) and Octave 4.0.0 (64-bits here), using the new GUI. I intend to port it to Linux (Ubuntu) in a near future.
