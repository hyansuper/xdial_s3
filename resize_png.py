import argparse
from PIL import Image

def resize_png(input_path, output_path, width, height):
    # Open the image
    with Image.open(input_path) as img:
        # Resize using high-quality resampling
        resized = img.resize((width, height), Image.LANCZOS)
        # Save as PNG
        resized.save(output_path, format='PNG')
        print(f"Resized image saved to {output_path} ({width}x{height})")

def main():
    parser = argparse.ArgumentParser(description="Resize a PNG image using Pillow.")
    parser.add_argument("input", help="Path to the input PNG file")
    parser.add_argument("-o", "--output", help="Path to save the resized PNG")
    parser.add_argument("-s", "--size", help="New size (widthxheight) in pixels")

    args = parser.parse_args()
    w, h = args.size.split('x')
    resize_png(args.input, args.output, int(w), int(h))

if __name__ == "__main__":
    main()
