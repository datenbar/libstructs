"""
Reads .tmpl files from templates/, replaces the @T@ placeholder with each
type listed in types.txt, and writes real .c/.h files into generated/.

Run this any time a template changes or a new type is added:
    python gen.py
"""

from pathlib import Path

STRUCTS = []

TEMPLATE_DIR = Path("templates")
OUT_DIR = Path("generated")
TYPES_FILE = Path("types.txt")


def load_types():
    if not TYPES_FILE.exists():
        return []
    return [
        line.strip() for line in TYPES_FILE.read_text().splitlines() if line.strip()
    ]


def generate():
    types = load_types()
    OUT_DIR.mkdir(exist_ok=True)

    if not STRUCTS:
        print("no structs listed in gen.py yet, nothing to generate")
        return

    for struct in STRUCTS:
        for ext in ("h", "c"):
            tmpl_path = TEMPLATE_DIR / f"{struct}.{ext}.tmpl"
            if not tmpl_path.exists():
                continue
            template = tmpl_path.read_text()
            for t in types:
                out = template.replace("@T@", t)
                out_path = OUT_DIR / f"{struct}_{t}.{ext}"
                out_path.write_text(out)
                print(f"generated {out_path}")


if __name__ == "__main__":
    generate()
