# Rubik's Cube Solver With Computer Vision

Take six photos of a scrambled Rubik's cube, one per face. A MATLAB script finds the cube in each
photo and reads the color of every sticker. A C++ program rebuilds the cube from those colors and
prints a sequence of moves that solves it.

<!-- TODO: demo recording and terminal screenshot go here (docs/media/) -->

## Background

This is a personal project out of my interest in Rubik's cubes and speedcubing.

I wrote the solver first while I was learning C++ and object-oriented programming during my
Programming II class my first year of undergrad. Because of this, I implemented classes like
`Queue`, `LinkedList` and an exception class instead of the standard library ones. I also modeled
every cell of the cube as an object and this helped me solidify what I was learning in class. **The
program solves the cube the way a person using the beginner's method would. It is not trying to
find the shortest solution, just a correct one.**

Later on during my undergrad I took a graduate Computer Vision class, where I took the opportunity
to expand the solver and also use it for a class project. I used what I learned in that class to
write **a MATLAB program that takes images of a physical cube and detects its state**, which could
then be fed into the solver. I also wrote a paper on the method and its limits
([docs/](docs/Rubiks-Cube-Recognition-and-Color-Classification.pdf)).

Then I joined the two. The detector writes a text file, and I added code to the solver that reads
that file and rebuilds the cube from it. By then I had a couple more years of C++, so that part
uses the standard library (`vector`, `unordered_map`, `pair`) rather than my own classes.

The solver, the detector, and the code joining them are hand-coded. In 2026 I came back to polish
it with Claude Code. This included a few targeted bug fixes, input validation, a verification
script, and the documentation.

## How it works

```
 six photos (yellow, orange, blue, red, green, white face)
      |
      v
 Detector/CVRubiksCube.m            grayscale, median filter, threshold, rotate 33 degrees,
      |                             Prewitt edges, Hough lines, pick the four corners, crop,
      |                             split into 3x3, sample pixels, nearest reference color
      v
 Detector/stickers.txt              6 lines x 9 color letters
      |
      v
 Solver/RubiksCubeSolver auto       identify each corner and edge piece by its colors,
      |                             rebuild the cube, solve it layer by layer,
      |                             cancel out redundant moves
      v
 a solution, e.g.  R2 U F2 D' F2 U L F' L' D' ...
```

The two programs are separate on purpose. Running the detector requires a MATLAB license and the
Image Processing Toolbox, while running the solver requires only a C++ compiler. Keeping them apart means
anyone can try the solver without installing MATLAB. The text file is the only thing they share.

The solver represents the cube as an unfolded 9x12 grid where each cell holds a number saying which
solved-state sticker is currently there. A face turn is a set of sticker swaps on that grid. The
solve runs the seven stages of the beginner's method (white cross, white corners, middle edges,
then the yellow layer in four steps), printing the grid after each piece is placed, and finishes
by merging adjacent moves on the same face (`F F` becomes `F2`, `R R'` cancels).

[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) goes through both halves in detail, including the
sticker file format and every file in the repository.

## Results and limitations

From the paper:

- **Color classification is 97% accurate** on faces the detector locates correctly (157 of 162
  stickers). The misses confused orange with red, and once yellow with orange.
- **Cube detection is not robust.** It expects the cube centered, edges parallel to the frame, a
  light background and good lighting, and it fails on photos that vary in scale, rotation,
  background or lighting. The thresholds are also tuned for 2560x1440 photos.
- **Classical image processing, no learned model.** Colors are matched against six fixed RGB
  values I calibrated by hand.
- **Runtime is about 25 seconds** for six faces in MATLAB, most of it drawing figures.

## Quick start

### Solver only (no MATLAB needed)

Requires `make` and a C++17 compiler.

```sh
cd Solver
make
./RubiksCubeSolver auto ../Detector/stickers-exampleSet.txt
```

That reads the detector's output for the six example photos in `Detector/Images/ExampleSet` and
solves that cube. You will see the solved cube, the scrambled cube, the grid after each step, and
at the end a line like:

```
Algorithm translated into Rubik's cube notation:
R2 U F2 D' F2 U L F' L' D' U L F' L' D' F' U L F' L' D2 ...
```

The notation is explained under "manual" below. Expect 120 to 180 moves. The beginner's method is
not short.

To solve a scramble you type yourself:

```sh
./RubiksCubeSolver manual
```

and enter a scramble as moves separated by spaces, for example `R U R' U' F2 D`. Each move is one
letter for the layer to turn, and a turn is a quarter turn clockwise as you look at that face:

| Letter | Layer |
|---|---|
| `F` | front face |
| `B` | back face |
| `U` | up (top) face |
| `D` | down (bottom) face |
| `L` | left face |
| `R` | right face |
| `E` | equator, the middle layer between up and down, turned the same direction as `D` (solutions only, see below) |

Add `'` for counterclockwise (`R'`) or `2` for a half turn (`R2`). So `R U R' U' F2 D` means:
right clockwise, top clockwise, right counterclockwise, top counterclockwise, front half turn,
bottom clockwise.

`E` shows up in the solver's output but is not accepted in a scramble. The solver assumes the
six centers start in their home positions, and an `E` in the scramble would move them somewhere
face turns alone cannot undo.

### Full pipeline (MATLAB required)

1. Take six photos, one per face, and name them `Img1.jpg` through `Img6.jpg` in this order:
   yellow, orange, blue, red, green, white (by center color). For the four side faces hold the cube
   with yellow on top. For the yellow face, green should be at the top of the photo, and for the
   white face, blue should be at the top.

   Photograph the cube close up, centered, with its edges parallel to the frame, on a plain light
   background under good light. `Detector/Images/ExampleSet` shows what works.

2. Put them in a folder under `Detector/Images/`, say `Detector/Images/MyCube/`.

3. In MATLAB, from the `Detector` folder:

   ```matlab
   CVRubiksCube("MyCube")
   ```

   It writes `Detector/stickers.txt`. If it could not find the cube in one of the photos, it says
   which one so you can retake it.

   To check your MATLAB setup first, run `CVRubiksCube()` with no arguments. It processes the
   example set, and the `stickers.txt` it writes should match `stickers-exampleSet.txt` exactly.

4. Solve it (using the solver built in the section above):

   ```sh
   cd Solver
   ./RubiksCubeSolver auto
   ```

## Verification

`tools/verify_solver.py` checks the solver against a separately written model of a Rubik's cube.
It generates random scrambles, runs the solver on each, applies the printed solution to its own
model, and confirms every face comes out a single color. It also runs the example sticker file
through `auto` mode the same way.

```sh
cd Solver && make && cd ..
python3 tools/verify_solver.py
```

## Repository layout

| Path                               | What it is                                               |
| ---------------------------------- | -------------------------------------------------------- |
| `Solver/`                          | The C++ solver. `make` builds `RubiksCubeSolver`.        |
| `Detector/CVRubiksCube.m`          | The MATLAB detector and color classifier.                |
| `Detector/Images/ExampleSet/`      | Six example photos of a real scrambled cube.             |
| `Detector/stickers-exampleSet.txt` | The detector's output for those photos.                  |
| `tools/verify_solver.py`           | Independent correctness check for the solver.            |
| `docs/ARCHITECTURE.md`             | How everything works, file by file.                      |
| `docs/*.pdf`                       | The paper on the detector's method, experiments and results. |
