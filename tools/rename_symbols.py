import argparse
import re
from typing import Dict, List, Tuple, Set


DEFINE_RE = re.compile(r"^\s*(.+?)\s*=\s*(.+?)\s*$")


def parse_map(items: List[str]) -> Dict[str, str]:
    """
    Parse --map OLD=NEW entries.
    """
    out: Dict[str, str] = {}
    for it in items:
        m = DEFINE_RE.match(it)
        if not m:
            raise SystemExit(f"Bad --map entry: {it}. Expected OLD=NEW.")
        old, new = m.group(1).strip(), m.group(2).strip()
        out[old] = new
    return out


def rename_tokens_in_code(text: str, mapping: Dict[str, str]) -> str:
    """
    Token-safe replacement of identifiers in non-string/comment regions.
    Only replaces whole identifier tokens (A-Za-z0-9_).
    """
    keys = set(mapping.keys())
    out: List[str] = []
    i = 0
    n = len(text)

    # States:
    # 0 normal
    # 1 line comment //
    # 2 block comment /* */
    # 3 string "..."
    # 4 char '...'
    state = 0

    def peek(offset: int = 0) -> str:
        idx = i + offset
        if idx < 0 or idx >= n:
            return ""
        return text[idx]

    while i < n:
        c = text[i]

        if state == 0:
            # comments
            if c == "/" and peek(1) == "/":
                state = 1
                out.append("//")
                i += 2
                continue
            if c == "/" and peek(1) == "*":
                state = 2
                out.append("/*")
                i += 2
                continue

            # strings/chars
            if c == '"':
                state = 3
                out.append(c)
                i += 1
                continue
            if c == "'":
                state = 4
                out.append(c)
                i += 1
                continue

            # identifier token
            if c.isalpha() or c == "_":
                start = i
                i += 1
                while i < n and (text[i].isalnum() or text[i] == "_"):
                    i += 1
                tok = text[start:i]
                out.append(mapping.get(tok, tok))
                continue

            out.append(c)
            i += 1
            continue

        if state == 1:
            # line comment
            out.append(c)
            i += 1
            if c == "\n":
                state = 0
            continue

        if state == 2:
            # block comment
            if c == "*" and peek(1) == "/":
                out.append("*/")
                i += 2
                state = 0
            else:
                out.append(c)
                i += 1
            continue

        if state == 3:
            # string
            out.append(c)
            if c == "\\":
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            i += 1
            if c == '"':
                state = 0
            continue

        if state == 4:
            # char literal
            out.append(c)
            if c == "\\":
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            i += 1
            if c == "'":
                state = 0
            continue

    return "".join(out)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", required=True)
    ap.add_argument("--inplace", action="store_true")
    ap.add_argument("--map", action="append", default=[], help="OLD=NEW mapping. May be repeated.")
    ap.add_argument("--mapfile", default=None, help="JSON file with mapping {OLD: NEW}.")
    args = ap.parse_args()

    mapping: Dict[str, str] = {}
    if args.mapfile:
        import json as _json
        with open(args.mapfile, "r", encoding="utf-8") as f:
            mapping = _json.load(f)
    if args.map:
        mapping.update(parse_map(args.map))
    if not mapping:
        raise SystemExit("Empty mapping. Provide --map or --mapfile.")

    with open(args.file, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    renamed = rename_tokens_in_code(text, mapping)

    if args.inplace:
        with open(args.file, "w", encoding="utf-8", errors="replace") as f:
            f.write(renamed)
    else:
        print(renamed)


if __name__ == "__main__":
    main()

