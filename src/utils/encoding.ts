import jschardet from 'jschardet';

// TODO: better ideas?
export function detectEncoding(array: Uint8Array) {
    const encoding = require('encoding-japanese');
    const str = encoding.convert(array, {
        to: 'UTF8',
        type: 'string'
    }) as Buffer;
    return jschardet.detect(str).encoding;
}