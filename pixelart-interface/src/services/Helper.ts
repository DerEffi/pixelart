export function rgb565(red: number, green: number, blue: number): number {
    return (((red & 0b11111000)<<8) + ((green & 0b11111100)<<3) + (blue>>3));
}

export function rgb888(rgb565: number): {r: number, g: number, b: number} {
    let r: number = ((((rgb565 >> 11) & 0x1F) * 527) + 23) >> 6;
    let b: number = ((((rgb565 >> 5) & 0x3F) * 259) + 33) >> 6;
    let g: number = (((rgb565 & 0x1F) * 527) + 23) >> 6;

    return {
        r: r,
        g: g,
        b: b
    };
}

export function c2dArray(pixels: number[][], hex: boolean = true): string {
    return JSON.stringify(hex ? pixels.map(row => row.map(pixel => "0x" + pixel.toString(16))) : pixels).replaceAll("[", "{").replaceAll("]", "}").replaceAll("\"", "");
}