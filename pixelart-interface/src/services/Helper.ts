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

//wrapper for awaiting timeout in async function
export async function asyncTimeout(ms: number): Promise<void> {
    return new Promise<void>(resolve => {
        setTimeout(() => resolve(), ms);
    });
}