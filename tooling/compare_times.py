#!/usr/bin/env python3
"""Compare wall-clock time and stack usage of ./anthropic and ./google as a
Sudoku gets harder.

Starts from a fully solved board, blanks one more random cell each round,
writes the board to the puzzle file, and times both solvers on it. Each run
is wrapped in `/usr/bin/time -l`, whose "maximum resident set size" is used
as a proxy for stack frame complexity: this recursive DFS solver's memory
growth is almost entirely the call stack, since the board itself is a fixed
9x9 array reused in place. A solver is dropped once it passes the cutoff;
the run ends when both have.

Both binaries are run with no arguments from their own directory, so each
run solves puzzle1.txt .. puzzle5.txt; only puzzle5.txt changes, and round 0
(no unknowns) gives the fixed baseline cost of the other four puzzles.
"""

import argparse
import csv
import os
import random
import re
import signal
import subprocess
import time
from pathlib import Path

SOLVED = """\
5 3 9 8 2 4 1 7 6
1 8 6 5 7 9 2 4 3
4 7 2 1 3 6 8 9 5
9 2 4 3 8 7 5 6 1
3 1 8 6 9 5 4 2 7
6 5 7 4 1 2 3 8 9
7 4 5 2 6 1 9 3 8
8 9 1 7 4 3 6 5 2
2 6 3 9 5 8 7 1 4
"""

SOLVERS = ["anthropic", "google"]
UNKNOWN = "_"
ROOT = Path(__file__).resolve().parent.parent
MAXRSS_RE = re.compile(rb"(\d+)\s+maximum resident set size")


def write_board(path, board):
    path.write_text("".join(" ".join(row) + " \n" for row in board))


def time_solver(exe, cwd, cutoff):
    """Run exe under `/usr/bin/time -l` and return (elapsed seconds, peak
    stack/heap bytes), or None if the solver passed the cutoff.

    Runs in its own process group so a timeout can kill the whole `time` +
    solver tree instead of leaving the solver running in the background.
    """
    proc = subprocess.Popen(
        ["/usr/bin/time", "-l", str(exe)], cwd=cwd,
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        start_new_session=True,
    )
    start = time.perf_counter()
    try:
        _, stderr = proc.communicate(timeout=cutoff)
    except subprocess.TimeoutExpired:
        os.killpg(proc.pid, signal.SIGKILL)
        proc.communicate()
        return None
    elapsed = time.perf_counter() - start

    match = MAXRSS_RE.search(stderr)
    max_rss = int(match.group(1)) if match else 0
    return elapsed, max_rss


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--dir", type=Path, default=ROOT / "exec" / "mac_arm",
                        help="directory holding the two binaries and the puzzle file")
    parser.add_argument("--puzzle", default="puzzle5.txt",
                        help="puzzle file (inside --dir) to overwrite each round")
    parser.add_argument("--cutoff", type=float, default=30.0,
                        help="seconds before a solver is stopped and dropped")
    parser.add_argument("--seed", type=int, default=None,
                        help="random seed, for a repeatable removal order")
    parser.add_argument("--csv", type=Path, default=Path(__file__).with_name("times.csv"),
                        help="where to save the results")
    args = parser.parse_args()

    seed = args.seed if args.seed is not None else random.randrange(2**32)
    rng = random.Random(seed)
    print(f"seed {seed}, cutoff {args.cutoff:g}s, puzzle {args.dir / args.puzzle}")

    board = [line.split() for line in SOLVED.splitlines()]
    cells = [(r, c) for r in range(9) for c in range(9)]
    rng.shuffle(cells)

    puzzle = args.dir / args.puzzle
    original = puzzle.read_bytes() if puzzle.exists() else None
    active = list(SOLVERS)
    rows = []

    # Two columns per solver: wall-clock seconds and peak RSS (KB), the
    # latter standing in for stack frame complexity (see module docstring).
    header_cols = [f"{s}_{suffix}" for s in SOLVERS for suffix in ("sec", "rss_kb")]
    print(f"{'unknowns':>8}  " + "  ".join(f"{h:>12}" for h in header_cols))
    try:
        for unknowns in range(len(cells) + 1):
            if unknowns:
                r, c = cells[unknowns - 1]
                board[r][c] = UNKNOWN
            write_board(puzzle, board)

            row = {"unknowns": unknowns}
            for solver in SOLVERS:
                sec_key, rss_key = f"{solver}_sec", f"{solver}_rss_kb"
                if solver not in active:
                    row[sec_key] = ""
                    row[rss_key] = ""
                    continue
                result = time_solver(args.dir / solver, args.dir, args.cutoff)
                if result is None:
                    active.remove(solver)
                    row[sec_key] = f">{args.cutoff:g}"
                    row[rss_key] = ""
                else:
                    elapsed, max_rss = result
                    row[sec_key] = f"{elapsed:.4f}"
                    row[rss_key] = f"{max_rss / 1024:.0f}"
            rows.append(row)
            print(f"{unknowns:>8}  " + "  ".join(f"{row[h] or '-':>12}" for h in header_cols),
                  flush=True)

            if not active:
                break
    finally:
        # Put the real puzzle back so the binaries behave normally afterwards.
        if original is not None:
            puzzle.write_bytes(original)
        else:
            puzzle.unlink(missing_ok=True)
        with args.csv.open("w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=["unknowns"] + header_cols)
            writer.writeheader()
            writer.writerows(rows)
        print(f"saved {args.csv}")


if __name__ == "__main__":
    main()
