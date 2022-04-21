import {detectEncoding} from './encoding';
import * as fs from 'fs';

describe('test encoding utils', () => {

    test('detectEncoding UTF-8', () => {
        const buf = fs.readFileSync('test/SubRIP UTF-8.srt');
        expect(detectEncoding(buf)).toEqual('UTF-8');
    });

    test('detectEncoding UTF-8 BOM', () => {
        const buf = fs.readFileSync('test/SubRIP BOM.srt');
        expect(detectEncoding(buf)).toEqual('UTF-8');
    });

    test('detectEncoding UTF-8 BOM', () => {
        const buf = fs.readFileSync('test/SubRIP BOM.srt');
        expect(detectEncoding(buf)).toEqual('UTF-8');
    });

    test('detectEncoding ASCII', () => {
        const buf = fs.readFileSync('test/SubRIP ASCII.srt');
        expect(detectEncoding(buf)).toEqual('windows-1251');
    });

    test('detectEncoding UTF-16', () => {
        const buf = fs.readFileSync('test/SubRIP UTF-16 LE.srt');
        expect(detectEncoding(buf)).toEqual('UTF-16LE');
    });
});