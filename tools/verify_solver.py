#!/usr/bin/env python3
"""Independently verify that RubiksCubeSolver's solutions actually solve the cube.

This script keeps its own, separately written model of a Rubik's cube (a plain 54-sticker
facelet cube), applies the scramble, applies the solution the solver printed, and checks that
every face is a single color.

Two checks are run:
  1. Manual mode: N random scrambles are generated, fed to `RubiksCubeSolver manual`, and the
     returned solution is applied to the scrambled cube.
  2. Auto mode: a detector output file (default: Detector/stickers-exampleSet.txt) is read into
     the facelet cube directly, fed to `RubiksCubeSolver auto <file>`, and the returned solution
     is applied.

Usage:
    python3 tools/verify_solver.py [path/to/RubiksCubeSolver] [N] [path/to/stickers.txt]

Defaults: Solver/RubiksCubeSolver, N=100, Detector/stickers-exampleSet.txt (relative to the repo root).
Exit status is 0 if every check passed, 1 otherwise. Requires only the Python standard library.
"""

import os
import random
import re
import subprocess
import sys

# ---------------------------------------------------------------------------------------------
# Facelet cube model
#
# Six faces, 9 stickers each, indexed 0..8 row-major as seen when looking straight at the face
# in the standard net (U above F, D below F, L-F-R-B left to right). Face letters follow the
# solver's convention: U=yellow, L=orange, F=blue, R=red, B=green, D=white.
# ---------------------------------------------------------------------------------------------

FACES = "ULFRBD"


def solved_cube():
    return {f: [f] * 9 for f in FACES}


def rotate_face_cw(a):
    return [a[6], a[3], a[0], a[7], a[4], a[1], a[8], a[5], a[2]]


# For each move: the four strips of three stickers that cycle when the face turns clockwise.
# Strip k receives the stickers that were in strip k-1.
STRIPS = {
    "U": [("B", [0, 1, 2]), ("R", [0, 1, 2]), ("F", [0, 1, 2]), ("L", [0, 1, 2])],
    "D": [("F", [6, 7, 8]), ("R", [6, 7, 8]), ("B", [6, 7, 8]), ("L", [6, 7, 8])],
    "F": [("U", [6, 7, 8]), ("R", [0, 3, 6]), ("D", [2, 1, 0]), ("L", [8, 5, 2])],
    "B": [("U", [2, 1, 0]), ("L", [0, 3, 6]), ("D", [6, 7, 8]), ("R", [8, 5, 2])],
    "R": [("U", [8, 5, 2]), ("B", [0, 3, 6]), ("D", [8, 5, 2]), ("F", [8, 5, 2])],
    "L": [("U", [0, 3, 6]), ("F", [0, 3, 6]), ("D", [0, 3, 6]), ("B", [8, 5, 2])],
    # E is the middle horizontal slice; it turns in the same direction as D and moves no face.
    "E": [("L", [3, 4, 5]), ("F", [3, 4, 5]), ("R", [3, 4, 5]), ("B", [3, 4, 5])],
}

TURNS = {"": 1, "2": 2, "'": 3}


def apply_move(cube, move):
    face, suffix = move[0], move[1:]
    for _ in range(TURNS[suffix]):
        if face != "E":
            cube[face] = rotate_face_cw(cube[face])
        strips = STRIPS[face]
        old = [[cube[f][i] for i in idx] for f, idx in strips]
        for k, (f, idx) in enumerate(strips):
            for i, v in zip(idx, old[(k - 1) % 4]):
                cube[f][i] = v


def apply_algorithm(cube, algorithm):
    for move in algorithm.split():
        apply_move(cube, move)


def is_solved(cube):
    return all(len(set(cube[f])) == 1 for f in FACES)


def self_test():
    """Sanity checks on the model itself, so a broken verifier can't pass a broken solver."""
    c = solved_cube()
    apply_algorithm(c, "R U R' U' " * 6)
    assert is_solved(c), "(R U R' U')x6 should be the identity"
    c = solved_cube()
    apply_algorithm(c, "F")
    assert not is_solved(c)
    assert c["U"][6:9] == ["L"] * 3 and c["R"][0] == "U" and c["D"][0:3] == ["R"] * 3 and c["L"][2] == "D"
    c = solved_cube()
    apply_algorithm(c, "E E E E")
    assert is_solved(c)


# ---------------------------------------------------------------------------------------------
# Running the solver
# ---------------------------------------------------------------------------------------------

ANSI = re.compile(r"\x1b\[[0-9;]*m")
SOLUTION_HEADER = "Algorithm translated into Rubik's cube notation:"


def run_solver(exe, args, stdin_text=None):
    result = subprocess.run([exe] + args, input=stdin_text, capture_output=True, text=True, timeout=60)
    output = ANSI.sub("", result.stdout)
    if result.returncode != 0:
        raise RuntimeError(f"solver exited with {result.returncode}: {result.stderr.strip() or output[-500:]}")
    if SOLUTION_HEADER not in output:
        raise RuntimeError("solver output did not contain a solution")
    solution = output.split(SOLUTION_HEADER, 1)[1].strip().splitlines()[0]
    return "" if solution.startswith("No moves") else solution


def random_scramble(rng):
    moves = [f + s for f in "FRUDLB" for s in ("", "2", "'")]
    return " ".join(rng.choice(moves) for _ in range(rng.randint(1, 25)))


def check_manual(exe, n, rng):
    failures = []
    for _ in range(n):
        scramble = random_scramble(rng)
        solution = run_solver(exe, ["manual"], stdin_text=scramble + "\n")
        cube = solved_cube()
        apply_algorithm(cube, scramble)
        apply_algorithm(cube, solution)
        if not is_solved(cube):
            failures.append((scramble, solution))
    return failures


def check_auto(exe, sticker_file):
    with open(sticker_file) as fh:
        rows = [line.split() for line in fh if line.strip()]
    if len(rows) != 6 or any(len(r) != 9 for r in rows):
        raise RuntimeError(f"{sticker_file}: expected 6 rows of 9 stickers")
    # Detector face order is yellow, orange, blue, red, green, white = U L F R B D.
    cube = {face: list(rows[i]) for i, face in enumerate(FACES)}
    solution = run_solver(exe, ["auto", sticker_file])
    apply_algorithm(cube, solution)
    return is_solved(cube), solution


def main(argv):
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    exe = argv[1] if len(argv) > 1 else os.path.join(repo_root, "Solver", "RubiksCubeSolver")
    n = int(argv[2]) if len(argv) > 2 else 100
    sticker_file = argv[3] if len(argv) > 3 else os.path.join(repo_root, "Detector", "stickers-exampleSet.txt")

    if not os.path.exists(exe):
        print(f"Solver binary not found at {exe}. Run `make` in Solver/ first.")
        return 1

    self_test()
    ok = True

    failures = check_manual(exe, n, random.Random(2019))
    print(f"manual mode: {n - len(failures)}/{n} random scrambles solved")
    for scramble, solution in failures[:5]:
        print(f"  FAIL scramble: {scramble}\n       solution: {solution}")
    ok &= not failures

    solved, solution = check_auto(exe, sticker_file)
    print(f"auto mode:   {os.path.relpath(sticker_file, repo_root)} -> {'solved' if solved else 'NOT solved'} ({len(solution.split())} moves)")
    ok &= solved

    print("PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
