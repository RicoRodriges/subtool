
export function downloadAsFile(data: string, filename: string) {
    const fileContent = new Blob([data], {type: 'text/plain'});
    if (window.navigator.msSaveOrOpenBlob) { // IE10+
        window.navigator.msSaveOrOpenBlob(fileContent, filename);
    } else {
        const a = document.createElement('a');
        const url = URL.createObjectURL(fileContent);
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        setTimeout(() => {
            document.body.removeChild(a);
            window.URL.revokeObjectURL(url);
        }, 0);
    }
}

export function readBytes(file: File, callback: (x: Uint8Array) => void) {
    const reader = new FileReader();
    reader.onloadend = (evt) => {
        if (evt.target && evt.target.readyState === FileReader.DONE) {
            callback(new Uint8Array(evt.target.result as ArrayBuffer));
        }
    };
    reader.readAsArrayBuffer(file);
}

export function readText(file: File, encoding: string, callback: (x: string) => void) {
    const reader = new FileReader();
    reader.onloadend = (evt) => {
        if (evt.target && evt.target.readyState === FileReader.DONE) {
            callback(evt.target.result as string);
        }
    };
    reader.readAsText(file, encoding);
}