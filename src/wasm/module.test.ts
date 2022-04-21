/**
 * @jest-environment node
 */
import '../../public/subtitles.js'
import {parseSubtitles} from './module';
import {EmscriptenSubtitle, ReturnCode} from './types';
import * as fs from 'fs';

describe('test wasm module', () => {
    function assertSubtitles(result: EmscriptenSubtitle | null, code: ReturnCode, type: string, lnEnding: boolean, hasStop = true) {
        expect(code).toEqual(ReturnCode.OK);
        expect(result?.type).toEqual(type);
        expect(result?.data.length).toEqual(2);
        expect(result?.data.map(v => [v.text, v.start, v.stop])).toEqual([
            [`<i>Line 1</i>${lnEnding ? '\n' : ''}`, (60 + 1) * 1000, hasStop ? ((60 + 2) * 1000) : -1],
            [`Line 🐷\nLine 3${lnEnding ? '\n' : ''}`, (60 + 3) * 1000, hasStop ? ((60 + 4) * 1000) : -1],
        ]);
    }

    test('blank content', async () => {
        const {result, code} = await parseSubtitles('');
        expect(code).toEqual(ReturnCode.ERROR);
        expect(result).toBeNull();
    });

    test('SubRIP UTF-8.srt', async () => {
        const f = fs.readFileSync('test/SubRIP UTF-8.srt').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SubRIP", true);
    });

    test('SubRIP BOM.srt', async () => {
        const f = fs.readFileSync('test/SubRIP BOM.srt').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SubRIP", true);
    });

    test('SubRIP UTF-16 LE.srt', async () => {
        const f = fs.readFileSync('test/SubRIP UTF-16 LE.srt').toString('utf16le');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SubRIP", true);
    });

    test('SubRIP EOF.srt', async () => {
        const f = fs.readFileSync('test/SubRIP EOF.srt').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SubRIP", true);
    });

    test('MicroDVD.sub', async () => {
        const f = fs.readFileSync('test/MicroDVD.sub').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "MicroDVD", false);
    });

    test('MicroDVD.sub custom fps', async () => {
        const f = fs.readFileSync('test/MicroDVD.sub').toString('utf8');
        const {result, code} = await parseSubtitles(f, 50);
        expect(result?.type).toEqual("MicroDVD");
        expect(result?.data.length).toEqual(2);
        expect(result?.data[0].start).toEqual((60 + 1) * 1000 / 2);
        expect(result?.data[0].stop).toEqual((60 + 2) * 1000 / 2);
    });

    test('SubViewer.txt', async () => {
        const f = fs.readFileSync('test/SubViewer.txt').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SubViewer", true);
    });

    test('SBV.txt', async () => {
        const f = fs.readFileSync('test/SBV.txt').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SBV", true);
    });

    test('ASS.ass', async () => {
        const f = fs.readFileSync('test/ASS.ass').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "SSA/ASS", false);
    });

    test('MPL.mpl', async () => {
        const f = fs.readFileSync('test/MPL.mpl').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, "MPL2", false);
    });

    test('VPlayer.tmp', async () => {
        const f = fs.readFileSync('test/VPlayer.tmp').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        assertSubtitles(result, code, 'VPlayer', false, false);
    });

    test('SAMI.smi', async () => {
        const f = fs.readFileSync('test/SAMI.smi').toString('utf8');
        const {result, code} = await parseSubtitles(f);
        expect(code).toEqual(ReturnCode.OK);
        expect(result?.type).toEqual('SAMI');
        expect(result?.data.length).toEqual(4);
        expect(result?.data.map(v => [v.text, v.start, v.stop])).toEqual([
            ['Line 1', 61000, -1],
            [' ', 62000, -1],
            ['Line 🐷\nLine 3', 63000, -1],
            [' ', 64000, -1],
        ]);
    });
});