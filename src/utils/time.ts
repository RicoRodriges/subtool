export function ToTimeString(time: number, delim: string, msPad = 3) {
    const ms = Math.floor(+(time % 1000).toPrecision(msPad)).toString().substring(0, msPad);
    const s = Math.floor(time / 1000) % 60;
    const m = Math.floor(time / 1000 / 60) % 60;
    const h = Math.floor(time / 1000 / 60 / 60);
    return `${pad(h, 2)}:${pad(m, 2)}:${pad(s, 2)}${delim}${pad(ms, msPad)}`
}

function pad(val: number | string, size: number, fill: string = '0') {
    return val.toString().padStart(size, fill);
}