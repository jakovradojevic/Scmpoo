import argparse
import re
from typing import Dict, Tuple, List, Set


IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*$")


def parse_alias_defines(text: str) -> Dict[str, str]:
    """
    Extract simple object-like macro aliases:
      #define OLD NEW
    where both OLD and NEW are identifiers.

    We only keep aliases whose OLD token starts with:
      - 'word_'
      - 'stru_'
    and also a legacy token 'ownerwindow' (if present).
    """
    aliases: Dict[str, str] = {}
    for line in text.splitlines():
        m = DEFINE_RE.match(line)
        if not m:
            continue
        old, new = m.group(1), m.group(2)
        if old.startswith("word_") or old.startswith("stru_") or old == "ownerwindow":
            aliases[old] = new
    return aliases


def resolve_aliases(aliases: Dict[str, str]) -> Dict[str, str]:
    """
    Resolve chains like word_A -> otherAlias -> finalName.
    Only resolves through known aliases.
    """
    resolved: Dict[str, str] = {}

    def resolve_one(x: str, seen: Set[str]) -> str:
        if x in resolved:
            return resolved[x]
        if x in seen:
            # Cycle (shouldn't happen in our case); stop.
            return aliases.get(x, x)
        seen.add(x)
        nxt = aliases.get(x)
        if nxt is None:
            resolved[x] = x
            return x
        final = resolve_one(nxt, seen)
        resolved[x] = final
        return final

    for k in aliases.keys():
        resolve_one(k, set())
    return resolved


def inline_aliases_in_code(text: str, alias_map: Dict[str, str]) -> str:
    """
    Replace identifier tokens (not substrings) in normal code regions only.
    Skips replacements inside:
      - double-quoted strings
      - single-quoted character literals
      - // line comments
      - /* block comments */
    """
    alias_keys = set(alias_map.keys())

    out: List[str] = []
    i = 0
    n = len(text)

    # States:
    # 0 normal
    # 1 line comment
    # 2 block comment
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
            # Start of comments?
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
            # Start of strings/chars
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

            # Identifier token?
            if c.isalpha() or c == "_":
                start = i
                i += 1
                while i < n and (text[i].isalnum() or text[i] == "_"):
                    i += 1
                tok = text[start:i]
                if tok in alias_keys:
                    out.append(alias_map[tok])
                else:
                    out.append(tok)
                continue

            out.append(c)
            i += 1
            continue

        if state == 1:
            # line comment until newline or EOF
            if c == "\n":
                state = 0
                out.append(c)
                i += 1
            else:
                out.append(c)
                i += 1
            continue

        if state == 2:
            # block comment until */
            if c == "*" and peek(1) == "/":
                state = 0
                out.append("*/")
                i += 2
            else:
                out.append(c)
                i += 1
            continue

        if state == 3:
            # string literal; handle escapes
            out.append(c)
            if c == "\\":
                # escape next char
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            if c == '"':
                state = 0
            i += 1
            continue

        if state == 4:
            # char literal; handle escapes
            out.append(c)
            if c == "\\":
                if i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                else:
                    i += 1
                continue
            if c == "'":
                state = 0
            i += 1
            continue

    return "".join(out)


def remove_alias_define_lines(text: str, old_aliases: Set[str]) -> str:
    """
    Remove lines that are exactly:
      #define OLD NEW
    where OLD is in old_aliases.
    """
    lines = text.splitlines(True)  # keepends
    out_lines: List[str] = []
    for line in lines:
        m = DEFINE_RE.match(line.rstrip("\r\n"))
        if m:
            old = m.group(1)
            if old in old_aliases:
                continue
        out_lines.append(line)
    return "".join(out_lines)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", required=True, help="Path to Scmpoo.c (or other C file).")
    ap.add_argument("--inplace", action="store_true", help="Rewrite the file in place.")
    args = ap.parse_args()

    path = args.file
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    aliases = parse_alias_defines(text)
    if not aliases:
        raise SystemExit("No alias #define(word_/stru_/ownerwindow) found.")

    alias_map = resolve_aliases(aliases)

    # Inline in code regions
    inlined = inline_aliases_in_code(text, alias_map)

    # Remove the alias define lines
    inlined = remove_alias_define_lines(inlined, set(aliases.keys()))

    if args.inplace:
        with open(path, "w", encoding="utf-8", errors="replace") as f:
            f.write(inlined)
    else:
        print(inlined)


if __name__ == "__main__":
    main()

