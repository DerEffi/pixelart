//schema for cache item to save image creation history
export interface ICachedImage {
    id: number;
    name: string;
    preview: string;
    created: number;
    animation: number[][][];
    image: number[];
}