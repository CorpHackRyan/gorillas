#!/usr/bin/env python3
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: embed_bgm.py <input_mp3> <output_c>", file=sys.stderr)
        return 1

    mp3_path = sys.argv[1]
    out_path = sys.argv[2]

    with open(mp3_path, "rb") as f:
        data = f.read()

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("#include <stddef.h>\n\n")
        f.write("const unsigned char _binary_brianD_starkiller_mp3_start[] = {\n")
        for i, b in enumerate(data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"{b},")
            if i % 16 == 15:
                f.write("\n")
        if len(data) % 16 != 0:
            f.write("\n")
        f.write("};\n")
        f.write(
            "const size_t _binary_brianD_starkiller_mp3_len = "
            "sizeof(_binary_brianD_starkiller_mp3_start);\n"
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
