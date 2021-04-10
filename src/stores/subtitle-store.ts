import {action, computed, observable} from "mobx";
import {readBytes, readText} from "../utils/file";
import {detectEncoding} from "../utils/encoding";
import {call, freeStr, mallocStr} from "../utils/emscripten";
import {Align, EmscriptenSubtitle, ReturnCode} from "../types";
import {ToTimeString} from "../utils/time";

export default class SubtitleStore {
    @observable firstSubtitle: Subtitle;
    @observable secondSubtitle: Subtitle;
    @observable align: Align;
    @observable synchronization: number;

    @computed
    public get mergeLines() {
        if (this.firstSubtitle.loaded && this.secondSubtitle.loaded) {
            return [...this.firstSubtitle.lines].concat(this.secondSubtitle.lines).sort(comparator);
        } else {
            return undefined;
        }
    }

    constructor() {
        this.firstSubtitle = new Subtitle();
        this.secondSubtitle = new Subtitle();
        this.align = Align.VERTICAL;
        this.synchronization = 0;
    }

    @action
    public async loadSubtitles(f: File, encoding = '', fps = 0, speed = 1,
                               f2?: File, encoding2 = '', fps2 = 0, speed2 = 1,
                               synchronization = 0) {
        this.firstSubtitle.reset();
        this.secondSubtitle.reset();
        return Promise.all([
            this.loadSubtitle(this.firstSubtitle, f, encoding, fps, speed),
            f2 ? this.loadSubtitle(this.secondSubtitle, f2, encoding2, fps2, speed, synchronization) : Promise.resolve()
        ]);
    }

    private async loadSubtitle(subtitle: Subtitle, f: File, encoding: string, fps: number,
                               speed: number, synchronization = 0) {
        return new Promise<string>(res => {
            if (encoding) {
                res(encoding);
                return;
            }
            readBytes(f, buf => {
                res(detectEncoding(buf));
            })
        }).then(detectedEncoding => new Promise<EmscriptenSubtitle>((res, rej) => {
            subtitle.encoding = detectedEncoding;
            readText(f, detectedEncoding, str => {
                // @ts-ignore: Clean before C code
                delete window.parsed;

                const content = mallocStr(str);
                const code = call<ReturnCode>('parse',
                    'number',
                    ['number', 'number', 'number'],
                    [content.content, content.size - 1, fps]);
                // @ts-ignore: Came from C code
                const result = window.parsed as EmscriptenSubtitle;
                freeStr(content);

                if (code === ReturnCode.OK) {
                    res(result);
                } else {
                    rej(code);
                }
            });
        })).then(s => {
            subtitle.parseCode = ReturnCode.OK;
            subtitle.fps = fps || undefined;
            subtitle.type = s.type;
            subtitle.lines = s.data.map(l => new SubtitleLine(l.start * speed + synchronization, l.stop * speed + synchronization, l.text.trim()))
                .filter(l => l.start >= 0)
                .sort(comparator);
        }, e => {
            subtitle.parseCode = e;
            return Promise.reject(e);
        });
    }

    public mergeToSubRip() {
        if (this.firstSubtitle !== undefined && this.secondSubtitle !== undefined) {
            const [firstSubtitleSection, secondSubtitleSection] = this.getScreenSections();
            return this.firstSubtitle.lines.map(l => new SubtitleLine(l.start, l.stop, `{\\an${firstSubtitleSection}}${l.line}`)).concat(
                this.secondSubtitle.lines.map(l => new SubtitleLine(l.start, l.stop, `{\\an${secondSubtitleSection}}${l.line}`))
            )
                .sort(comparator)
                .map((l, i) => l.toSubRipLine(i + 1))
                .join('\n');
        } else {
            return undefined;
        }
    }

    public mergeToASS() {
        if (this.firstSubtitle !== undefined && this.secondSubtitle !== undefined) {
            const [firstSubtitleSection, secondSubtitleSection] = this.getScreenSections();
            return '[Script Info]\n' +
                'ScriptType: v4.00+\n' +
                'Collisions: Normal\n' +
                'PlayDepth: 0\n' +
                'Timer: 100,0000\n' +
                'Video Aspect Ratio: 0\n' +
                'WrapStyle: 0\n' +
                'ScaledBorderAndShadow: no\n' +
                '\n' +
                '[V4+ Styles]\n' +
                'Format: Name,Fontname,Fontsize,PrimaryColour,SecondaryColour,OutlineColour,BackColour,Bold,Italic,Underline,StrikeOut,ScaleX,ScaleY,Spacing,Angle,BorderStyle,Outline,Shadow,Alignment,MarginL,MarginR,MarginV,Encoding\n' +
                'Style: Default,Arial,16,&H00FFFFFF,&H00FFFFFF,&H00000000,&H00000000,-1,0,0,0,100,100,0,0,1,3,0,2,10,10,10,0\n' +
                `Style: S1,Arial,16,&H00F9FFFF,&H00FFFFFF,&H00000000,&H00000000,-1,0,0,0,100,100,0,0,1,3,0,${firstSubtitleSection},10,10,10,0\n` +
                `Style: S2,Arial,16,&H00F9FFF9,&H00FFFFFF,&H00000000,&H00000000,-1,0,0,0,100,100,0,0,1,3,0,${secondSubtitleSection},10,10,10,0\n` +
                '\n' +
                '[Events]\n' +
                'Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n' +
                (
                    this.firstSubtitle.lines.map(l => ({line: l, style: 'S1'})).concat(
                        this.secondSubtitle.lines.map(l => ({line: l, style: 'S2'}))
                    )
                        .sort((o1, o2) => comparator(o1.line, o2.line))
                        .map((o) => o.line.toASSDialogue(o.style))
                        .join('\n')
                );
        } else {
            return undefined;
        }
    }

    private getScreenSections() {
        // Screen sections (from SSA specification):
        // _________
        // | 7 8 9 |
        // | 4 5 6 |
        // | 1 2 3 |
        // _________
        switch (this.align) {
            case Align.VERTICAL:
                return [2, 8];
            case Align.HORIZONTAL_TOP:
                return [7, 9];
            case Align.HORIZONTAL_BOTTOM:
                return [1, 3];
        }
    }
}

export class Subtitle {
    @observable encoding: string;
    @observable type: string;
    @observable lines: SubtitleLine[];
    @observable fps?: number;
    @observable parseCode?: ReturnCode;

    @computed get firstUnit() {
        return this.lines.length > 0 ? ToTimeString(this.lines[0].start, ',') : "None";
    }

    @computed get lastUnit() {
        return this.lines.length > 0 ? ToTimeString(this.lines[this.lines.length - 1].start, ',') : "None";
    }

    @computed
    public get loaded() {
        return this.parseCode === ReturnCode.OK;
    }

    constructor() {
        this.encoding = '';
        this.type = '';
        this.lines = [];
    }

    @action
    public reset() {
        this.encoding = '';
        this.type = '';
        this.lines = [];
        this.fps = undefined;
        this.parseCode = undefined;
    }

    public toSubRip() {
        return this.lines.map((l, i) => l.toSubRipLine(i + 1)).join('\n');
    }
}

export class SubtitleLine {
    @observable start: number;
    @observable stop: number;
    @observable line: string;

    constructor(start: number, stop: number, line: string) {
        this.start = start;
        this.stop = stop;
        this.line = line;
    }

    public toSubRipLine(i: number) {
        return `${i}\n` +
            `${ToTimeString(this.start, ',')} --> ${ToTimeString(this.stop, ',')}\n` +
            `${this.line.trim()}\n`;
    }

    public toASSDialogue(style: string) {
        const text = this.line.trim().split('\n').join('\\N');
        // Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
        return `Dialogue: 0,${ToTimeString(this.start, '.', 2)},${ToTimeString(this.stop, '.', 2)},${style},,0000,0000,0000,,${text}`;
    }
}

function comparator(l1: SubtitleLine, l2: SubtitleLine) {
    return l1.start - l2.start;
}