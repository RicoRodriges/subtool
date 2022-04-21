import {computed, makeObservable, observable, runInAction} from 'mobx';
import {readBytes, readText} from '../utils/file';
import {detectEncoding} from '../utils/encoding';
import {Align} from '../types';
import {ToTimeString} from '../utils/time';
import {ReturnCode} from '../wasm/types';
import {parseSubtitles} from '../wasm/module';

export type SubtitleFile = {
    f: File,
    encoding?: string,
    fps?: number, // (0; +oo)
    speed?: number, // (0; +oo)
    offset?: number,
};

export default class SubtitleStore {
    public firstSubtitle?: Subtitle | BadSubtitle;
    public secondSubtitle?: Subtitle | BadSubtitle;
    public align: Align;

    constructor() {
        this.align = Align.VERTICAL;

        makeObservable(this, {
            firstSubtitle: observable,
            secondSubtitle: observable,
            align: observable,
            mergeLines: computed,
        });
    }

    public get mergeLines() {
        if (this.firstSubtitle?.isOk() && this.secondSubtitle?.isOk()) {
            return [...this.firstSubtitle.lines].concat(this.secondSubtitle.lines).sort(SubtitleLine.comparator);
        }
        return undefined;
    }

    public async loadSubtitles(s1: SubtitleFile, s2?: SubtitleFile) {
        const subtitles = await Promise.all([
            this.loadSubtitle(s1),
            s2 ? this.loadSubtitle(s2) : Promise.resolve(undefined),
        ]);
        runInAction(() => {
            this.firstSubtitle = subtitles[0];
            this.secondSubtitle = subtitles[1];
        });
    }

    private async loadSubtitle(s: SubtitleFile): Promise<Subtitle | BadSubtitle> {
        const detectedEncoding = s.encoding || detectEncoding(await readBytes(s.f));

        const text = await readText(s.f, detectedEncoding);
        const r = await parseSubtitles(text, s.fps);

        if (r.code === ReturnCode.OK) {
            const fps = s.fps;
            const type = r.result.type;
            const speed = s.speed || 1;
            const offset = s.offset || 0;

            const lines = r.result.data.map(l => new SubtitleLine(l.start * speed + offset, l.stop > 0 ? (l.stop * speed + offset) : -1, l.text.trim()))
                .filter(l => l.start >= 0)
                .sort(SubtitleLine.comparator);
            lines.reduceRight((next, current) => {
                current.stop = current.stop > 0
                    ? current.stop
                    : next.start;
                return current;
            });
            return new Subtitle(detectedEncoding, type, lines.filter(l => l.line.length > 0), fps);
        }
        console.error('Subtitle was not loaded!');
        return new BadSubtitle(detectedEncoding);
    }

    public mergeToSubRip() {
        if (this.firstSubtitle?.isOk() && this.secondSubtitle?.isOk()) {
            const [firstSubtitleSection, secondSubtitleSection] = this.getScreenSections();
            return this.firstSubtitle.lines.map(l => new SubtitleLine(l.start, l.stop, `{\\an${firstSubtitleSection}}${l.line}`)).concat(
                this.secondSubtitle.lines.map(l => new SubtitleLine(l.start, l.stop, `{\\an${secondSubtitleSection}}${l.line}`))
            )
                .sort(SubtitleLine.comparator)
                .map((l, i) => l.toSubRipLine(i + 1))
                .join('\n');
        }
        return undefined;
    }

    public mergeToASS() {
        if (this.firstSubtitle?.isOk() && this.secondSubtitle?.isOk()) {
            const [firstSubtitleSection, secondSubtitleSection] = this.getScreenSections();
            const s1 = 'S1';
            const s2 = 'S2';
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
                `Style: ${s1},Arial,16,&H00F9FFFF,&H00FFFFFF,&H00000000,&H00000000,-1,0,0,0,100,100,0,0,1,3,0,${firstSubtitleSection},10,10,10,0\n` +
                `Style: ${s2},Arial,16,&H00F9FFF9,&H00FFFFFF,&H00000000,&H00000000,-1,0,0,0,100,100,0,0,1,3,0,${secondSubtitleSection},10,10,10,0\n` +
                '\n' +
                '[Events]\n' +
                'Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n' +
                (
                    this.firstSubtitle.lines.map(l => ({line: l, style: s1})).concat(
                        this.secondSubtitle.lines.map(l => ({line: l, style: s2}))
                    )
                        .sort((o1, o2) => SubtitleLine.comparator(o1.line, o2.line))
                        .map((o) => o.line.toASSDialogue(o.style))
                        .join('\n')
                );
        }
        return undefined;
    }

    private getScreenSections() {
        // Screen sections (from SSA specification):
        //  -------
        // | 7 8 9 |
        // | 4 5 6 |
        // | 1 2 3 |
        //  -------
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

export class BadSubtitle {
    public readonly encoding: string;

    constructor(encoding: string) {
        this.encoding = encoding;
    }

    public isOk(): this is Subtitle {
        return false;
    }
}

export class Subtitle {
    constructor(
        public readonly encoding: string,
        public readonly type: string,
        public readonly lines: SubtitleLine[],
        public readonly fps?: number,
    ) {
        makeObservable(this, {
            lines: observable,
            firstUnit: computed,
            lastUnit: computed,
        });
    }

    public get firstUnit() {
        return this.lines.length > 0
            ? ToTimeString(this.lines[0].start, ',', 3)
            : "None";
    }

    public get lastUnit() {
        return this.lines.length > 0
            ? ToTimeString(this.lines[this.lines.length - 1].start, ',', 3)
            : "None";
    }

    public isOk(): this is Subtitle {
        return true;
    }

    public toSubRip() {
        return this.lines.map((l, i) => l.toSubRipLine(i + 1)).join('\n');
    }
}

export class SubtitleLine {
    constructor(public start: number, public stop: number, public line: string) {
        makeObservable(this, {
            start: observable,
            stop: observable,
            line: observable,
        });
    }

    public toSubRipLine(i: number) {
        return `${i}\n` +
            `${ToTimeString(this.start, ',', 3)} --> ${ToTimeString(this.stop, ',', 3)}\n` +
            `${this.line.trim()}\n`;
    }

    public toASSDialogue(style: string) {
        const text = this.line.trim().split('\n').join('\\N');
        // Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
        return `Dialogue: 0,${ToTimeString(this.start, '.', 2)},${ToTimeString(this.stop, '.', 2)},${style},,0000,0000,0000,,${text}`;
    }

    public static comparator(l1: SubtitleLine, l2: SubtitleLine) {
        return l1.start - l2.start;
    }
}
