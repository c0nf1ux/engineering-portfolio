"""Writes the key file used by `abcu_benchmark --keys`.

Reads the `printings` table of a local SQLite database built from Scryfall bulk
data (my mtg-sorter project) and writes one tab-separated line per distinct
printing: "<set code>-<collector number>" and the card name. Keys are
de-duplicated case-insensitively and written in scryfall_id order, which is
effectively random with respect to the key text, so neither structure gets a
pre-sorted insertion order.

    python extract_scryfall_keys.py path/to/printings.db scryfall_keys.tsv
"""
import sqlite3
import sys


def main(db_path, out_path):
    connection = sqlite3.connect(f"file:{db_path}?mode=ro", uri=True)
    seen = {}
    query = ("SELECT name, set_code, collector_number FROM printings "
             "ORDER BY scryfall_id")
    for name, set_code, collector_number in connection.execute(query):
        key = f"{set_code}-{collector_number}"
        seen.setdefault(key.upper(), (key, name.replace("\t", " ")))
    with open(out_path, "w", encoding="utf-8", newline="\n") as out:
        for key, name in seen.values():
            out.write(f"{key}\t{name}\n")
    print(f"{len(seen)} unique keys written to {out_path}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
