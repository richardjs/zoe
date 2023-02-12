import { Player, Type } from "./enum.js";
import { HEX_SIZE } from "./hex.js";
import { Piece } from "./piece.js";
import { e } from "./shortcuts.js";

function draw(state, canvas) {
  const grid = [];

  // Convert state (string) into Pieces in grid
  // grid is indexed double-height coordinates:
  // https://www.redblobgames.com/grids/hexagons/#coordinates-doubled

  let minX = Infinity;
  let maxX = 0;
  let minY = Infinity;
  let maxY = 0;

  for (let i = 0; i + 2 < state.length; i += 3) {
    let piece = Piece.fromChar(state[i]);
    let q = state[i + 1].charCodeAt(0) - "a".charCodeAt(0);
    let r = state[i + 2].charCodeAt(0) - "a".charCodeAt(0);

    let x = q;
    let y = 2 * r + q;

    if (grid[x] === undefined) {
      grid[x] = [];
    }

    if (!grid[x][y]) {
      grid[x][y] = piece;
    } else {
      let p = grid[x][y];
      while (p.on_top) {
        p = p.on_top;
      }
      p.on_top = piece;
    }

    minX = Math.min(minX, x);
    maxX = Math.max(maxX, x);
    minY = Math.min(minY, y);
    maxY = Math.max(maxY, y);
  }

  // Resize canvas based on grid size

  let gridWidth = maxX - minX + 1;
  let gridHeight = maxY - minY + 1;

  canvas.width = 2 * HEX_SIZE + 1.5 * HEX_SIZE * (gridWidth - 1);
  canvas.height =
    HEX_SIZE * Math.sqrt(3) + 0.5 * HEX_SIZE * Math.sqrt(3) * (gridHeight - 1);

  // Draw grid

  const ctx = canvas.getContext("2d");

  // Hexes are drawn in pairs:
  //   1. A hex at an even x and y
  //   2. The hex to the southeast of it (x+1, y+1)
  for (let x = minX - (minX % 2); x <= maxX + (maxX % 2); x += 2) {
    for (let y = minY - (minY % 2); y <= maxY + (maxY % 2); y += 2) {
      ctx.save();

      ctx.translate(
        HEX_SIZE + (x - minX) * 1.5 * HEX_SIZE,
        0.5 * HEX_SIZE * Math.sqrt(3) +
          (y - minY) * 0.5 * HEX_SIZE * Math.sqrt(3)
      );

      if (grid[x] && grid[x][y]) {
        let piece = grid[x][y];
        piece.draw(ctx);
      }

      ctx.translate(1.5 * HEX_SIZE, 0.5 * HEX_SIZE * Math.sqrt(3));

      if (grid[x + 1] && grid[x + 1][y + 1]) {
        let piece = grid[x + 1][y + 1];
        piece.draw(ctx);
      }

      ctx.restore();
    }
  }
}

export default function Grid({ state }) {
  const canvasRef = React.useRef(null);

  function handleClick(e) {
    const canvas = canvasRef.current;
    const x = e.clientX - canvas.getBoundingClientRect().left;
    const y = e.clientY - canvas.getBoundingClientRect().top;

    // https://www.redblobgames.com/grids/hexagons/#pixel-to-hex
    let q = Math.floor(((2 / 3) * x) / HEX_SIZE);
    let r = Math.floor(((-1 / 3) * x + (Math.sqrt(3) / 3) * y) / HEX_SIZE);
  }

  React.useEffect(() => {
    draw(state, canvasRef.current);
  }, [state]);

  return e("canvas", {
    className: "grid",
    onClick: handleClick,
    ref: canvasRef,
  });
}
