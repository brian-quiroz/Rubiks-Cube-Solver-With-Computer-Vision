# Architecture

This document explains how the two halves of the project fit together, how each one works
internally, and what every file in the repository is for. The [README](../README.md) is the
short version. This is the reference.

## 1. Rubik's cube terms used in this document

If you already speak cube, skip to §2.

**Faces and centers.** A 3x3x3 Rubik's cube has six faces. Each face has a center sticker, and the
six centers are fixed relative to each other. No move can swap two centers. That is why a face can
be named by a color even when the cube is scrambled. "The yellow face" means "the face whose center
is yellow", and the eight stickers around that center may be any colors at all until the cube is
solved.

**Pieces and stickers.** The cube is made of 26 small cubes (cubies). There are 6 centers (one
sticker each, fixed), 12 edges (two stickers each), and 8 corners (three stickers each). A move
carries whole pieces around, and the stickers on a piece stay together. So an edge that shows yellow
and blue is the same physical piece wherever it is on the cube. Both halves of this project rely on
that fact, because a piece is identified by the set of colors on it.

**Face letters.** Moves are written relative to the person holding the cube: F (front), B (back),
U (up), D (down), L (left), R (right). In this project the cube is always held with yellow on top
and blue in front, so U is the yellow face, D white, F blue, B green, L orange, R red.

The same six faces also get numbered 1 to 6 in the order the detector photographs them and writes
them to the sticker file: yellow, orange, blue, red, green, white. So one face ends up with three
names depending on which part of the project is talking about it. The yellow face is "U" in the
solver, "face 1" (or `c[0]`) in the sticker file and the reconstruction code, and "the yellow
face" in the README. This table is the whole mapping:

| Color of center | Solver letter | Sticker file line | Photo |
|---|---|---|---|
| yellow | U | 1 | Img1.jpg |
| orange | L | 2 | Img2.jpg |
| blue | F | 3 | Img3.jpg |
| red | R | 4 | Img4.jpg |
| green | B | 5 | Img5.jpg |
| white | D | 6 | Img6.jpg |

**Move notation.** A face letter alone means turn that face 90 degrees clockwise as you look at it
(F, R, U, ...). A prime means counterclockwise (F', R'). A 2 means a half turn (F2, R2). E is a
slice move. It turns the middle horizontal layer, the one between U and D, in the same direction
as D. The solver uses E in its solutions but does not accept it in a scramble. §3 Moves explains
why.

**Algorithm.** In cube vocabulary an "algorithm" is any fixed sequence of moves, for example
`R U R' U'`. A scramble is an algorithm that messes the cube up, and a solution is one that solves
it.

**Permutation and orientation.** A piece is in the right *permutation* when it is in the correct
slot, and in the right *orientation* when it is also turned the right way in that slot. A corner in
the right slot can still be twisted one of three ways, and an edge can be flipped. "Solved" means every
piece has both right.

**Beginner's method.** The layer-by-layer method most people learn first. Solve a cross on one
face, then the four corners of that layer, then the four edges of the middle layer, then the last
layer in stages. Each stage has essentially one algorithm (a few have a mirrored or repeated
variant), so the whole method is a handful of short sequences. Advanced methods like CFOP
(Fridrich) trade that simplicity for speed and need 70-plus memorized algorithms. Each algorithm
scrambles pieces temporarily while it runs, but by the time it finishes, everything solved in
earlier stages is back where it was. That is what lets the stages be done one after another.

## 2. Data flow

```
 Detector/Images/<set>/Img1..6.jpg          six photos, one per face, in the order
          |                                 yellow, orange, blue, red, green, white
          v
 Detector/CVRubiksCube.m   (MATLAB)         locate the cube in each photo, crop it,
          |                                 split it into 9 stickers, classify each color
          v
 Detector/stickers.txt                      6 lines x 9 letters, e.g.  B B G R Y R G Y G
          |
          v
 Solver/RubiksCubeSolver auto <file>  (C++) rebuild the cube from the letters,
          |                                 solve it with the beginner's method
          v
 a move sequence, e.g.  R2 U F2 D' F2 U L F' L' ...
```

The two programs are run separately, and the text file is the only interface between them. There is
no launcher that chains them, by design. The detector requires a MATLAB license and the Image
Processing Toolbox, the solver requires only a C++ compiler, and keeping them independent means
anyone can try the solver without installing MATLAB.

### The sticker file format

Six lines, one per face, each holding nine single-letter colors separated by spaces. The letters
are `Y O B R G W`. Faces appear in the fixed order **yellow, orange, blue, red, green, white**,
where "yellow face" means the face with the yellow center (see §1). Within a face the nine
stickers are listed row by row, top-left first, *as the face appears in the unfolded net below*.
`Detector/stickers-exampleSet.txt` is a complete example. Its first line is the yellow face, and
the fifth letter of every line (the center) is always that face's color.

The solver treats the six faces as **U L F R B D** respectively, so the net looks like this:

```
              +---------+
              |  Y (U)  |
  +---------+-+---------+-+---------+---------+
  |  O (L)  |    B (F)    |  R (R)  |  G (B)  |
  +---------+-+---------+-+---------+---------+
              |  W (D)  |
              +---------+
```

This fixes the orientation each face must be photographed in:

| Face (by center color) | Hold the cube so that... |
|---|---|
| orange, blue, red, green (the sides) | the yellow center is on top |
| yellow (U) | the green center is at the top of the photo |
| white (D) | the blue center is at the top of the photo |

If the sticker counts are right but a face is photographed in the wrong orientation, the solver
rejects the file ("does not exist on a real cube") rather than produce a wrong answer, because the
stickers it reads as one corner will not be a color combination any real corner has.

## 3. Solver internals (`Solver/`)

### The cube representation

The cube is stored as a 9 x 12 grid of `Piece` objects, which is the unfolded net above drawn on a
rectangle. Only 54 of the 108 cells are real stickers, and the rest stay blank (value 0). Each real
cell holds a **permutation number** from 1 to 54 that identifies *which solved-state sticker is
currently sitting there*. (The name comes from the code. It is a sticker ID, not "permutation" in
the §1 sense.)

```
                 col:  0  1  2 |  3  4  5 |  6  7  8 |  9 10 11
                              |  1  2  3 |
  rows 0-2  (U)               |  4  5  6 |
                              |  7  8  9 |
            -----------------------------------------------------
                      10 11 12 | 13 14 15 | 16 17 18 | 19 20 21
  rows 3-5 (L F R B)  22 23 24 | 25 26 27 | 28 29 30 | 31 32 33
                      34 35 36 | 37 38 39 | 40 41 42 | 43 44 45
            -----------------------------------------------------
                              | 46 47 48 |
  rows 6-8  (D)               | 49 50 51 |
                              | 52 53 54 |
```

In the solved state cell *n* holds *n*. After a scramble, reading the grid tells you where every
sticker went. For example, if cell 3 (top-right of U) holds 19, the sticker that belongs at the
top-left of the green face is currently at the top-right of the yellow face. `printCube()` prints
this grid with ANSI colors (yellow, magenta, cyan, red, green, white for U L F R B D). That is the
colored output you see in the terminal.

### Moves

When you turn a face, every sticker on it moves to another position on that same face, and the
stickers along the four adjacent faces shift over by one face. In the grid that is a handful of
"rotate these four cells" operations, where sticker A moves to where B was, B to where C was, C to
where D was, and D to where A was. Each such loop of four is one `temp = a; a = b; b = c; c = d; d = temp`
block in the code.

`innerMovement(row, col)` does two of those loops for the face itself (one for its four edge
stickers, one for its four corner stickers). The face function (`F()`, `R()`, and so on) then does
three more loops for the stickers on the neighboring faces, one for the edges and two for the
corners. `E()` is the middle slice, so it has no face of its own and only does the three neighbor
loops, including the four centers. It is the one move that relocates centers on the grid, and that
is why a scramble may not contain it. The solving stages only turn faces (their own `E'` calls come
in groups of four that cancel), so if a scramble leaves the centers rotated, no sequence of face
turns brings them home and the last stage never terminates. Counterclockwise and half turns are
just repetition, so `Fp()` is three `F()`s and `F2()` is two. Every call also appends its name to the
`solverAlg` queue, which is how the solution is recorded.

Because the representation is a plain grid and moves are literal sticker swaps, the model is easy
to check by hand and easy to verify against an independent model (see `tools/verify_solver.py`).

### Solving

`Solver::Solver()` builds a solved cube, applies the scramble, then runs the seven stages of the
beginner's method in order, printing the grid after each piece is placed:

| Stage | Method | Goal | Helper algorithms |
|---|---|---|---|
| 1 | `Cross()` | white cross on D | `flipEdge` |
| 2 | `FirstLayerF2L()` | white corners | `insertRightCorner`, `insertLeftCorner`, `insertTopCorner`, `removeCornerU(p)` |
| 3 | `SecondLayerF2L()` | middle-layer edges | `insertAtLeft`, `insertAtRight`, `removeAtLeft`, `removeAtRight` |
| 4 | `EdgesOLL()` | orient yellow edges (yellow cross) | `FRURUF` |
| 5 | `EdgesPLL()` | permute yellow edges | `train` |
| 6 | `CornersPLL()` | permute yellow corners | `inAndOut` |
| 7 | `CornersOLL()` | orient yellow corners | `RDRD` |

Each stage is a lookup, not a search. It finds where the target piece currently is
(`foundInferiorEdge`, `foundInferiorCorner`, `foundMiddleEdge`, ...) and applies the canned
algorithm for that location. Stages 1 to 3 handle one piece at a time, turning D or E between pieces so the
same algorithm can be reused for each of the four. It runs instantly.

### Simplification

`simplifyAlgorithm()` converts the move queue into an internal notation (`F` becomes `F1`, `F2`
stays `F2`, `F'` becomes `F3`), copies it into a `LinkedList`, and repeatedly merges adjacent moves
on the same face by adding their counts mod 4 (`F1 F1` becomes `F2`, `R1 R3` cancels out). The same
routine is applied to the user's scramble before it is executed. The raw solution is usually 250 to
300 moves, and after simplification 120 to 180.

### Reconstructing a cube from sticker colors

This is the code I added when I joined the two projects. It lives at the top of `Solver.cpp`
(`autoScramble`, `identifyPieces`, `identifyCorner`, `identifyEdge`, and the `*StickerPerm*`
helpers). The solver thinks in sticker numbers (which solved sticker is in each cell), but the
detector hands over colors. This code turns one into the other.

The key idea is that a piece is identified by its colors. If some corner slot on the scrambled
cube shows yellow, blue and red, that piece *is* the yellow-blue-red corner, and there is exactly
one place it belongs on a solved cube. So for every slot, I read the colors there, work out which
piece that is, and therefore know which sticker numbers must be in that slot.

1. `autoScramble()` reads the 6 x 9 letters, checks that they make sense (only the six letters,
   exactly nine of each), and hands them to `identifyPieces()`.

2. `identifyPieces()` goes slot by slot. A corner slot is three grid cells that touch at a corner
   of the cube, and an edge slot is two cells. There are 8 corner slots and 12 edge slots, and
   `stdCornersCoords` / `stdEdgesCoords` list their grid coordinates. `cornersGiven` /
   `edgesGiven` list, in the same order, the letters from the sticker file that currently sit in
   each slot. So entry 3 of `stdCornersCoords` is where corner slot 3 is on the grid, and entry 3
   of `cornersGiven` is what colors are showing there right now.

3. For each slot, `identifyCorner()` (or `identifyEdge()`) figures out which piece is there. It
   compares the letters it was given against the list of the 8 corners a real cube has (`{Y,O,G}`,
   `{Y,R,G}`, and so on). A piece can sit in a slot several ways (a corner can be twisted three
   ways, an edge flipped two), so it tries each ordering of the letters (`cornerStickerPerm1..5`,
   `edgeStickerPerm`) until one matches. It returns which piece it found and which ordering
   matched, and the ordering is the same as saying how the piece is twisted. If nothing matches, the
   colors are not a real piece (a sticker was misread) and it reports an error.

4. Now each slot is known: "slot 3 holds the piece that belongs in slot 6, twisted like so."
   From that I can pair up every cell in slot 3 with the sticker number that lives in the
   matching cell of slot 6. Those pairs go into `old2NewStickers`, and a new grid is filled in
   from the map. Centers map to themselves, because centers never move.

From that point on the solver has no idea the cube came from photographs.

### Support classes

`Queue`, `LinkedList` and `Node` are my own singly-linked templates, used in place of `std::queue`
and `std::list`. `PreconditionViolationException` is my own exception type, thrown when a container
is misused (dequeue on an empty queue, index out of range). `Piece` wraps a single permutation
number in a class. I wrote the solver while learning C++ and object-oriented programming, and
implementing these myself was part of that. I have kept them as they were.

## 4. Detector internals (`Detector/CVRubiksCube.m`)

The detector is classical image processing with no learned model. It runs the same pipeline on each
of the six photos. I tuned the constants below by hand for my original photos (2560 x 1440, phone
camera, light background, lamp overhead). The paper in `docs/` explains each choice.

**Cube detection** (`detectEdges`, `plotEdges`, `detectCorners`, `findROIs`)

| Step | MATLAB | Constant | Why |
|---|---|---|---|
| Grayscale | `rgb2gray` | | edges don't need color |
| Denoise | `medfilt2` | 3 x 3 | remove sensor noise, keep edges sharp |
| Binarize | `imbinarize` | t = 0.25 | dark cube body vs light background |
| Rotate | `imrotate` | 33 degrees | breaks the alignment between the cube's edges and the image axes so the Hough transform gives clean peaks |
| Edges | `edge(..., 'prewitt')` | | simple gradient operator, enough after thresholding |
| Lines | `hough`, `houghpeaks`, `houghlines` | 15 peaks, FillGap 182.5, MinLength 150 | find long straight segments |
| Filter | theta/rho ranges, 230 < x < 2330 | | drop image-border lines and outliers (values assume 2560-px width) |
| Corners | min/max x and y among segment endpoints | | the four extreme endpoints are the cube's corners |
| Sanity check | MSE of the four side lengths vs. the longest line | abort if >= 10 000 | if the corners don't form a square, detection failed, so stop rather than guess |
| Crop | un-rotate the corners, `imcrop` | | save one square face image to `CroppedImages/` |

**Color classification** (`splitIntoStickers`, `detectColor`)

| Step | Constant | Why |
|---|---|---|
| Split the crop into a 3 x 3 grid | inset 60 px per cell | avoid the black plastic borders |
| Sample pixels | 50 random points per cell, `medfilt3` first | cheap and robust to specular highlights |
| Drop dark pixels | R + G + B < 80 | ignore any remaining border |
| Take the median RGB | | one representative color per sticker |
| Nearest reference color | squared distance to 6 hand-calibrated RGBs | red, green, blue, orange, yellow, white become `R G B O Y W` |

Output: `stickers.txt` in the format described in §2.

### What the paper found

`docs/Rubiks-Cube-Recognition-and-Color-Classification.pdf` (13 pages) documents the design and
the experiments:

- **Color classification: 97%.** 157 of 162 stickers correct across 18 correctly-detected faces.
  Four of the five errors confused orange for red, one confused yellow for orange.
- **Cube detection is not robust** to changes in scale, translation, rotation, background, or
  lighting. It was tested on each of those factors and generally fails unless the photo matches
  the "standard" setup (cube centered, edges parallel to the frame, light background, good light).
  Adjusting the threshold `t` recovers some cases.
- **Hard-coded constants** tie it to 2560 x 1440 images. Other resolutions produce no lines.
- **Runtime is about 25 s** for six faces (about 20 s detection, 5 s classification), most of it
  spent drawing and saving intermediate figures.

## 5. File-by-file

| Path | Purpose |
|---|---|
| `README.md` | Overview, quick start, results and limitations |
| `.gitignore` | Build output, detector output, Python bytecode, OS files |
| `Solver/main.cpp` | CLI: argument parsing, scramble validation, error reporting |
| `Solver/Solver.h`, `Solver/Solver.cpp` | The solver: representation, moves, stages, simplifier, and the sticker-file reconstruction |
| `Solver/Piece.h`, `Solver/Piece.cpp` | One grid cell: holds a permutation number |
| `Solver/Node.h`, `Solver/Node.hpp` | Linked-list node template |
| `Solver/Queue.h`, `Solver/Queue.hpp` | Queue template: the scramble and solution move lists |
| `Solver/LinkedList.h`, `Solver/LinkedList.hpp` | List template: working storage for the simplifier |
| `Solver/PreconditionViolationException.h`, `.cpp` | Exception thrown by the containers on misuse |
| `Solver/Makefile` | `make` builds `RubiksCubeSolver`, `make clean` removes output |
| `Detector/CVRubiksCube.m` | The MATLAB detector and classifier (§4) |
| `Detector/Images/ExampleSet/Img1..6.jpg` | A real cube photographed correctly: the sample input |
| `Detector/stickers-exampleSet.txt` | The detector's output for `ExampleSet`, also the solver-only demo input |
| `Detector/stickers.txt` | *(generated)* detector output for the last run |
| `Detector/CroppedImages/` | *(generated)* the six cropped faces from the last run |
| `tools/verify_solver.py` | Independent correctness check for the solver (§6) |
| `docs/ARCHITECTURE.md` | This file |
| `docs/Rubiks-Cube-Recognition-and-Color-Classification.pdf` | The detector paper |

## 6. Verification

`tools/verify_solver.py` contains its own small model of a Rubik's cube, six faces of nine
stickers with each move written out as sticker swaps, independent of the solver's 9 x 12 grid.
It generates random scrambles, runs `RubiksCubeSolver manual` on each, applies the printed solution
to its own model, and checks that every face is a single color. It does the same for the
`ExampleSet` sticker file through `auto` mode, reading the file into its own model directly, so
that check also covers the reconstruction code in `identifyPieces`.

```sh
cd Solver && make && cd ..
python3 tools/verify_solver.py            # 100 random scrambles + the example set
python3 tools/verify_solver.py Solver/RubiksCubeSolver 500
```

## 7. Provenance

I wrote the solver and the detector as two separate projects a few years apart and later merged
them here. What changed in the merge:

- **Solver.** Unchanged from its original version except for `main.cpp`, `Solver.h` and
  `Solver.cpp`, where I added the `auto` mode and the reconstruction code described in §3.
  Everything from `simplifyAlgorithm` onward in `Solver.cpp` is the original.
- **Detector.** `CVRubiksCube.m` originally also contained the experiment harness that produced
  the paper's Appendix A. I removed it and simplified the paths. `ExampleSet` and
  `stickers-exampleSet.txt` are the original sample run. The experiment image sets are not
  included, but their conclusions are in the paper.
