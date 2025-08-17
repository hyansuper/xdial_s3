import argparse
from pathlib import Path

def find_matching_files(folder: Path, pattern: str, recursive: bool):
    if recursive:
        matched_files = folder.rglob(pattern)
    else:
        matched_files = folder.glob(pattern)
    return matched_files

def write_c_header_file(output_path: Path, filenames: list[str], include_files: str = None, declare: str = "*"):
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("#pragma once\n")
        f.write("// Auto-generated list of files (filenames without extensions):\n\n")

        # Optional include
        if include_files:
            for include_file in include_files.split(' '):
                f.write(f'#include "{include_file}"\n')

        # Commented list of filenames
        for name in filenames:
            f.write(declare.replace("*", name) + "\n")

def main():
    parser = argparse.ArgumentParser(description="Generate a C header file from matching filenames.")
    parser.add_argument("folder", type=str, help="Path to the folder.")
    parser.add_argument("pattern", type=str, help="Filename pattern (e.g., '*.txt', 'data_*.csv').")
    parser.add_argument("-r", "--recursive", action="store_true", help="Search recursively in subfolders.")
    parser.add_argument("-o", "--output", type=str, required=True, help="Path to output header file (e.g., files.h).")
    parser.add_argument("-i", "--include", type=str, help="Header file to include (e.g., common.h).")
    parser.add_argument("-d", "--declare", type=str, help="declare macro (e.g., LV_IMAGE_DECLARE(*);).")

    args = parser.parse_args()
    folder_path = Path(args.folder)
    output_path = Path(args.output)

    if not folder_path.is_dir():
        print(f"Error: '{folder_path}' is not a valid directory.")
        return

    matched_files = find_matching_files(folder_path, args.pattern, args.recursive)
    filenames = sorted(file.stem for file in matched_files)  # sort alphabetically

    write_c_header_file(output_path, filenames, args.include, args.declare)
    print(f"Header file written to: {output_path} ({len(filenames)} entries)")

if __name__ == "__main__":
    main()
