import re
import json
from typing import Dict, List, Tuple, Optional


FUNC_DEF_RE = re.compile(
    r"^\s*(?:"
    r"LRESULT\s+CALLBACK|"
    r"BOOL\s+CALLBACK|"
    r"void|int|WORD|DWORD|UINT|"
    r"HPALETTE|HBITMAP|HBRUSH|HGLOBAL|HWND"
    r")\b[^;{]*?\b(sub_[0-9A-Fa-f]{1,6})\b\s*\(",
    re.MULTILINE,
)

COMMENT_RE = re.compile(r"/\*[\s\S]*?\*/", re.MULTILINE)


NOISE_TOKENS = {
    "the",
    "a",
    "an",
    "and",
    "or",
    "to",
    "of",
    "in",
    "on",
    "for",
    "with",
    "by",
    "from",
    "at",
    "into",
    "each",
    "on",
    "sub",
    "unused",
}


def extract_adjacent_comment(text: str, match_start: int) -> Optional[str]:
    """
    Find the closest /* ... */ comment block that ends right before the match,
    allowing only whitespace in between.
    """
    last_comment = None
    for m in COMMENT_RE.finditer(text):
        if m.end() <= match_start:
            last_comment = (m.start(), m.end())
        else:
            break
    if not last_comment:
        return None
    c_start, c_end = last_comment
    between = text[c_end:match_start]
    if between.strip():
        return None
    return text[c_start:c_end]


def is_probably_definition(text: str, sig_end: int) -> bool:
    """
    Heuristic: if a { appears before a ; within a small lookahead, treat as definition.
    """
    lookahead = text[sig_end:sig_end + 800]
    brace = lookahead.find("{")
    semi = lookahead.find(";")
    if brace == -1:
        return False
    if semi == -1:
        return True
    return brace < semi


def comment_to_identifier(comment: str) -> str:
    # Strip /* */
    inner = comment.strip()
    if inner.startswith("/*"):
        inner = inner[2:]
    if inner.endswith("*/"):
        inner = inner[:-2]
    inner = inner.replace("\n", " ").replace("\r", " ")
    # Keep alnum as tokens
    tokens = re.findall(r"[A-Za-z0-9]+", inner)
    tokens2 = []
    for t in tokens:
        low = t.lower()
        if low in NOISE_TOKENS:
            continue
        tokens2.append(t)
    if not tokens2:
        tokens2 = ["Func"]
    # PascalCase
    parts = []
    for t in tokens2:
        if t.isdigit():
            parts.append(t)
        else:
            parts.append(t[:1].upper() + t[1:].lower())
    name = "".join(parts)
    # Ensure starts with a letter
    if not name or not name[0].isalpha():
        name = "Func" + name
    return name


def main() -> None:
    path = "Scmpoo/Scmpoo.c"
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    # Collect only likely function definitions, not prototypes.
    sigs = []
    for m in FUNC_DEF_RE.finditer(text):
        sub = m.group(1)
        # m.end() is after "(" due to regex ending, so the signature really ends later.
        # We'll use m.end() as a safe point; heuristic checks for { before ; shortly after.
        if is_probably_definition(text, m.end()):
            sigs.append((sub, m.start()))
    subs = sorted({s for s, _ in sigs})
    mapping: Dict[str, str] = {}
    collisions: Dict[str, List[str]] = {}

    for sub in subs:
        # Use the first definition signature match for that sub.
        mm = None
        for m2 in FUNC_DEF_RE.finditer(text):
            if m2.group(1) == sub and is_probably_definition(text, m2.end()):
                mm = m2
                break
        if mm is None:
            continue
        # Find comment immediately preceding the signature (whitespace-only in-between).
        comment = extract_adjacent_comment(text, mm.start())
        if comment is None:
            # Fallback: use the nearest preceding comment block within the last ~400 chars
            # (even if not perfectly adjacent), to still get a human-readable name.
            last = None
            for cm in COMMENT_RE.finditer(text):
                if cm.end() <= mm.start():
                    last = cm
                else:
                    break
            if last is None:
                continue
            if mm.start() - last.end() <= 400:
                comment = text[last.start():last.end()]
            else:
                continue
        new_name = comment_to_identifier(comment)
        mapping[sub] = new_name
        collisions.setdefault(new_name, []).append(sub)

    # Ensure uniqueness by suffixing when needed
    used = set()
    final: Dict[str, str] = {}
    counter = {}
    for old, new in sorted(mapping.items(), key=lambda kv: kv[0]):
        base = new
        if new not in used:
            final[old] = new
            used.add(new)
            continue
        counter[base] = counter.get(base, 1) + 1
        final[old] = f"{base}{counter[base]}"
        used.add(final[old])

    print("Total unique sub_ identifiers in file (definitions only):", len(subs))
    print("Generated mappings (with comments):", len(final))
    # Show a sample
    sample = sorted(final.items())[:30]
    for old, new in sample:
        print(f"{old} -> {new}")

    # Save mapping for potential reuse
    with open("tools/sub_rename_mapping.json", "w", encoding="utf-8") as f:
        json.dump(final, f, indent=2, ensure_ascii=False)
    print("Wrote tools/sub_rename_mapping.json")


if __name__ == "__main__":
    main()

