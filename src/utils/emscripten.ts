

export function mallocStr(str: string): HeapBuffer {
    // @ts-ignore
    const lengthBytes = lengthBytesUTF8(str) + 1;
    // @ts-ignore
    const stringOnWasmHeap = _malloc(lengthBytes);
    // @ts-ignore
    stringToUTF8(str, stringOnWasmHeap, lengthBytes);
    return {content: stringOnWasmHeap, size: lengthBytes};
}

export function freeStr(buf: HeapBuffer) {
    // @ts-ignore
    _free(buf.content);
}

export function call<T>(funcName: string, returnType: string,
                        inputTypes: string[], inputs: any[]) {
    // @ts-ignore
    return Module.ccall(funcName, returnType, inputTypes, inputs);
}

export interface HeapBuffer {
    content: any;
    size: number;
}