import jschardet from 'jschardet';

// TODO: better ideas?
export function detectEncoding(array: Uint8Array) {
    let limitedArray = array;
    if (limitedArray.length > 5000) {
        limitedArray = new Uint8Array(array.buffer, 0, 5000);
    }
    // @ts-ignore
    const str = String.fromCharCode.apply(null, limitedArray as number[]);
    return jschardet.detect(str).encoding;
}