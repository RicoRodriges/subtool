import {Align} from "../../types";

export interface IFormValueStore {
    file1: File | null;
    fps1: number | null;
    encoding1: string;
    speed1: number | null;

    singleMode: boolean;

    file2: File | null;
    fps2: number | null;
    encoding2: string;
    speed2: number | null;
    synchronization: number;
    align: Align;
}

export function createFormValueStore(): IFormValueStore {
    return {
        file1: null,
        fps1: null,
        encoding1: '',
        speed1: null,
        singleMode: true,
        file2: null,
        fps2: null,
        encoding2: '',
        speed2: null,
        synchronization: 0,
        align: Align.VERTICAL,
    };
}