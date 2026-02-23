from PIL import Image
import sys

def rgb888_to_rgb565(r, g, b):
    # Convert 8-bit RGB to 16-bit RGB565
    return ((r & 0xF8) << 8) | \
           ((g & 0xFC) << 3) | \
           (b >> 3)

def convert_png_to_raw(input_png, output_raw):
    img = Image.open(input_png).convert("RGB")
    width, height = img.size
    pixels = img.load()

    with open(output_raw, "wb") as out:
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                out.write(rgb565.to_bytes(2, byteorder="little"))

    print(f"Converted {input_png} ({width}x{height}) → {output_raw}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python png2raw.py input.png output.raw")
    else:
        convert_png_to_raw(sys.argv[1], sys.argv[2])
