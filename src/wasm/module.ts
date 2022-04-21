/// <reference types="emscripten" />
/** Above will import declarations from @types/emscripten, including Module etc. */
import {EmscriptenSubtitle, ReturnCode} from "./types";

// -s EXPORTED_RUNTIME_METHODS='[lengthBytesUTF8,stringToUTF8]'
// -s EXPORTED_FUNCTIONS='[_parse]'
interface MyEmscriptenModule extends EmscriptenModule {
    lengthBytesUTF8: typeof lengthBytesUTF8;
    stringToUTF8: typeof stringToUTF8;

    _parse(content: number, size: number, fps: number): ReturnCode;
}
// -sMODULARIZE -s EXPORT_NAME='createMyModule'
declare function createMyModule(o ?: object): Promise<MyEmscriptenModule>;

const module = createMyModule({
    print: function (text: string) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        console.log(text);
    },
    printErr: function (text: string) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        console.error(text);
    }
});


type HeapPointer = number;

type HeapItem = {
    content: HeapPointer;
    size: number;
}

function allocStr(m: MyEmscriptenModule, str: string): HeapItem {
    const lengthBytes = m.lengthBytesUTF8(str) + 1;
    const stringOnWasmHeap = m._malloc(lengthBytes);
    m.stringToUTF8(str, stringOnWasmHeap, lengthBytes);
    return {content: stringOnWasmHeap, size: lengthBytes};
}

function freeStr(m: MyEmscriptenModule, buf: HeapItem) {
    m._free(buf.content);
}

export async function parseSubtitles(content: string, fps?: number) {
    const m = await module;

    // @ts-ignore: Clean before C code
    delete globalThis.wasm_parsed;

    let code: ReturnCode;
    const ptr = allocStr(m, content);
    try {
        code = m._parse(ptr.content, ptr.size - 1, fps || 0);
    } finally {
        freeStr(m, ptr);
    }

    // @ts-ignore: Came from C code
    const result = globalThis.wasm_parsed as EmscriptenSubtitle;

    if (code === ReturnCode.OK) {
        return {result, code};
    } else {
        return {result: null, code};
    }
}
