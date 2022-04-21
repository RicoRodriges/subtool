# Subtitle tool

This project helps to combine two subtitles into a single file.

Demo is [here](https://ricorodriges.github.io/subtool)

## How it works

* Powered by [VLC media player](https://github.com/videolan/vlc) and [libass](https://github.com/libass/libass)
  libraries. It lets support about all known subtitle formats.
* WebAssembly and Emscripten. Everything works in your browser.
* React, mobx, Bootstrap. Simple, but powerful user interface.

## Compile

First of all you need to compile C code with VLC and libass libraries. Install Emscripten
and execute script below:
```sh
mkdir build
cd build

emcmake cmake ..
emmake make

cd ..
rm -r build
```

It creates `subtitle.js` and `subtitle.wasm` files in `public` directory.

Patch `subtitle.js` file:

```diff
- var createMyModule = ...
+ globalThis.createMyModule = ...
```

You may run tests from `src/wasm` directory to make sure WebAssembly code is ok.

Now you are ready to transpile TypeScript and React code:
* Run http://localhost:3000 server `npm start`
* Or build project for future deploy `npm run build`
