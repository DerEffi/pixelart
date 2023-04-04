//converting rgb888 (3byte) color codes to rgb565 (2byte)
export function rgb565(red: number, green: number, blue: number): number {
    return (((red & 0b11111000)<<8) + ((green & 0b11111100)<<3) + (blue>>3));
}

//converting rgb565 (2byte) color codes back to rgb888 (3byte)
export function rgb888(rgb565: number): {r: number, g: number, b: number} {
    return {
        r: (((rgb565 & 0xF800) >> 11) << 3),
        g: (((rgb565 & 0x7E0) >> 5) << 2),
        b: (((rgb565 & 0x1F)) << 3)
    };
}

//convert 2d pixel array to 2d array in c/c++ source code
export function c2dArray(pixels: number[][], hex: boolean = true): string {
    return JSON.stringify(hex ? pixels.map(row => row.map(pixel => "0x" + pixel.toString(16))) : pixels).replaceAll("[", "{").replaceAll("]", "}").replaceAll("\"", "");
}

//convert 3d animation array to 3d array in c/c++ source code
export function cAnimationArray(animation: number[][][], hex: boolean = true): string {
    return JSON.stringify(hex ? animation.map(frame => frame.map(change => [change[0], change[1], "0x" + change[2].toString(16)])) : animation).replaceAll("[", "{").replaceAll("]", "}").replaceAll("\"", "");
}

//wrapper for awaiting timeout in async function
export async function asyncTimeout(ms: number): Promise<void> {
    return new Promise<void>(resolve => {
        setTimeout(() => resolve(), ms);
    });
}

//converting numbers to *zero*-padded string
export function padLeft(number: number, length: number, character: string = '0'): string {
    let result = String(number);
    for (let i = result.length; i < length; ++i) {
      result = character + result;
    }
    return result;
};

//check for valid ssid input with regex
export function validateSSID(ssid: string) {
    return /^[^!#;+\]\/"\t][^+\]\/"\t]{0,30}[^ +\]\/"\t]$|^[^ !#;+\]\/"\t]$[ \t]+$/.test(ssid); //eslint-disable-line no-useless-escape
}


//check for valid wpa passphrase with regex
export function validateWPA(passphrase: string) {
    return /^[\u0020-\u007e\u00a0-\u00ff]{8,62}$/g.test(passphrase); //eslint-disable-line no-useless-escape
}

export function convertToFoldername(name: string) {
    return name.replaceAll(/[^ \w\.!@#$^+=-]/g, "_").replaceAll(/ {2}/g, " ").substring(0, 240).replaceAll(/^[ .]/g, "_").replaceAll(/[ .]$/g, "_"); //eslint-disable-line no-useless-escape
}

export function scrollIntoView(id: string) {
    if(id) {
        let element = document.getElementById(id);
        if(element) {
            element.scrollIntoView();
        }
    }
}