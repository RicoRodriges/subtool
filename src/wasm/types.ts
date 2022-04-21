export interface EmscriptenSubtitle {
    type: string;
    data: EmscriptenSubtitleLine[];
}

export interface EmscriptenSubtitleLine {
    start: number; // ms
    stop: number | -1; // ms
    text: string;
}

export enum ReturnCode {
    OK = 0,
    ERROR = -1,
    MALLOC_ERROR = -2,
}