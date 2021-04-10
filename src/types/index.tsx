export interface EmscriptenSubtitle {
    type: string;
    data: EmscriptenSubtitleLine[];
}

export interface EmscriptenSubtitleLine {
    start: number;
    stop: number;
    text: string;
}

export enum ReturnCode {
    OK = 0,
    ERROR = -1,
    MALLOC_ERROR = -2,
    FPS_NOT_SET = -3,
    UNKNOWN_SUBTITLE_FORMAT = -4
}

export enum Align {
    VERTICAL = 'Vertical',
    HORIZONTAL_TOP = 'Top',
    HORIZONTAL_BOTTOM = 'Bottom'
}