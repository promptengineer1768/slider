# Developer Notes

## Runtime shape

- `src/core/` owns the puzzle model, solvability rules, solver, scrambler, and save-file validation.
- `src/app/` owns the wxWidgets UI, animation, sound playback, and window chrome.
- `GameController` bridges those layers by owning the current `Board`, theme selection, and save/load/scramble/solve orchestration.

## Theme loading

- Themes are loaded from `resources/themes.json` at startup.
- The shipped package keeps the JSON beside the executable so the UI can be reskinned without recompiling.
- If the JSON is missing or malformed, the controller falls back to the built-in theme list.

## Save files

- Save/load functions return pair-based results so callers get a boolean success value and a concrete error string without optional out-parameters.
- The loader enforces a small maximum file size to keep malformed or accidental large files from being accepted.

## Solver notes

- The solver uses A* with Manhattan distance plus linear conflict.
- Heuristic values are cached per `BoardState` to avoid recomputing the same state repeatedly during a search.
- The node expansion limit is a safety guard, not a correctness boundary.

## Board caching

- `BoardState` maintains a tile-position cache so `Board::GetDirectionToMoveTile()` can resolve a tile in constant time.
- The cache is rebuilt on construction and updated on every tile swap.
