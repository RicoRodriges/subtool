
export function downloadAsFile(data: string, filename: string) {
    const url = URL.createObjectURL(new Blob([data], {type: 'text/plain'}));
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    setTimeout(() => {
        document.body.removeChild(a);
        window.URL.revokeObjectURL(url);
    }, 0);
}

export async function readBytes(file: Blob) {
    return new Promise<Uint8Array>((r, rej) => {
        const reader = new FileReader();
        reader.onloadend = (evt) => {
            if (evt.target?.readyState === FileReader.DONE) {
                r(new Uint8Array(evt.target.result as ArrayBuffer));
            } else {
                rej();
            }
        };
        reader.readAsArrayBuffer(file);
    });
}

export async function readText(file: Blob, encoding: string) {
    return new Promise<string>((r, rej) => {
        const reader = new FileReader();
        reader.onloadend = (evt) => {
            if (evt.target?.readyState === FileReader.DONE) {
                r(evt.target.result as string);
            } else {
                rej();
            }
        };
        reader.readAsText(file, encoding);
    });
}