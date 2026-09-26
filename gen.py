"""
Reads .tmpl files from templates/, replaces the @T@ placeholder with each
type listed in types.txt, and writes real .c/.h files into generated/.

Run this any time a template changes or a new type is added:
    python gen.py

Dependency resolution
----------------------
DEPENDENCIES below records that graph once. Requesting a struct in STRUCTS
automatically pulls in everything it depends on, transitively, before
generating anything, so the generated .c file's #include "vector_int.h"
always has something real to point at.
"""

from pathlib import Path

# Which structures to generate. Add a name here once its .tmpl files exist
# in templates/ , dependencies are handled automatically.
STRUCTS = ["vector"]

DEPENDENCIES = {
    "vector": [],
}

TEMPLATE_DIR = Path("templates")
OUT_DIR = Path("generated")
TYPES_FILE = Path("types.txt")


def load_types():
    if not TYPES_FILE.exists():
        return []
    return [
        line.strip() for line in TYPES_FILE.read_text().splitlines() if line.strip()
    ]


def resolve_dependencies(requested):
    """Expand a list of requested struct names into itself plus every
    dependency, transitively, with duplicates removed. Order doesn't matter
    for compilation since #include handles ordering, this just needs to
    make sure nothing required is left out."""
    resolved = set()

    def visit(name):
        if name in resolved:
            return
        if name not in DEPENDENCIES:
            print(
                f"warning: '{name}' has no entry in DEPENDENCIES, "
                f"assuming it depends on nothing"
            )
            resolved.add(name)
            return
        resolved.add(name)
        for dep in DEPENDENCIES[name]:
            visit(dep)

    for name in requested:
        visit(name)

    return resolved


def generate():
    types = load_types()
    OUT_DIR.mkdir(exist_ok=True)

    all_structs = resolve_dependencies(STRUCTS)
    added = all_structs - set(STRUCTS)
    if added:
        print(f"auto-generating dependencies too: {sorted(added)}")

    for struct in sorted(all_structs):
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
