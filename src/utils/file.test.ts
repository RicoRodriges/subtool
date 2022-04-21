import {readBytes, readText} from "./file";
import * as fs from "fs";

describe('test file utils', () => {

    test('readBytes', async () => {
        const b = await readBytes(new Blob(['123'], {type: 'plain/text'}));
        expect(b).toEqual(new Uint8Array([49, 50, 51]));
    });

    test('readText', async () => {
        const t = await readText(new Blob(['123'], {type: 'plain/text'}), 'UTF-8');
        expect(t).toEqual('123');
    });

    test('readText file BOM', async () => {
        const f = fs.readFileSync('test/SubRIP BOM.srt');
        const t = await readText(new Blob([f], {type: 'plain/text'}), 'UTF-8');
        // has no BOM
        expect(t.at(0)).toEqual('1');
        expect(t).toMatch(/.*Line 1.*/);
        expect(t).toMatch(/.*Line 🐷.*/);
    });

    test('readText file UTF-8', async () => {
        const f = fs.readFileSync('test/SubRIP UTF-8.srt');
        const t = await readText(new Blob([f], {type: 'plain/text'}), 'UTF-8');
        expect(t.at(0)).toEqual('1');
        expect(t).toMatch(/.*Line 1.*/);
        expect(t).toMatch(/.*Line 🐷.*/);
    });

    test('readText file UTF-16', async () => {
        const f = fs.readFileSync('test/SubRIP UTF-16 LE.srt');
        const t = await readText(new Blob([f], {type: 'plain/text'}), 'UTF-16LE');
        expect(t.at(0)).toEqual('1');
        expect(t).toMatch(/.*Line 1.*/);
        expect(t).toMatch(/.*Line 🐷.*/);
    });

    test('readText file ASCII', async () => {
        const f = fs.readFileSync('test/SubRIP ASCII.srt');
        const t = await readText(new Blob([f], {type: 'plain/text'}), 'windows-1251');
        expect(t.at(0)).toEqual('1');
        expect(t).toMatch(/.*Line 1.*/);
        expect(t).toMatch(/.*Привет мир.*/);
    });

});