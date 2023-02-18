import { Player, Type } from "./enum.js";
import { HEX, HEX_SIZE } from "./hex.js";
import { axialToDoubleHeight, pixelToAxial } from "./util.js";
import { Piece } from "./piece.js";
import { e } from "./shortcuts.js";
import { doubleHeightToString } from "./util.js";

// Margin around the rendered hexes, to allow space for move input
const EDGE_SPACE = HEX_SIZE;

// TODO Make this user-controlable
const DRAW_COORDS = false;

function draw(canvas, state, highlight) {
  const grid = [];

  // Convert state (string) into Pieces in grid
  // grid is indexed double-height coordinates:
  // https://www.redblobgames.com/grids/hexagons/#coordinates-doubled

  let minX = Infinity;
  let maxX = 0;
  let minY = Infinity;
  let maxY = 0;

  for (let i = 0; i + 2 < state.length; i += 3) {
    const piece = Piece.fromChar(state[i]);
    const q = state.charCodeAt(i + 1) - "a".charCodeAt(0);
    const r = state.charCodeAt(i + 2) - "a".charCodeAt(0);

    const { x, y } = axialToDoubleHeight({ q, r });

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

  canvas.width =
    2 * HEX_SIZE + 1.5 * HEX_SIZE * (gridWidth - 1) + 2 * EDGE_SPACE;
  canvas.height =
    HEX_SIZE * Math.sqrt(3) +
    0.5 * HEX_SIZE * Math.sqrt(3) * (gridHeight - 1) +
    2 * EDGE_SPACE;

  // Draw grid

  const ctx = canvas.getContext("2d");

  ctx.translate(EDGE_SPACE, EDGE_SPACE);

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
      if (highlight && x == highlight.x && y == highlight.y) {
        ctx.strokeStyle = "#0d0";
        ctx.lineWidth = 3;
        ctx.stroke(HEX);
      }
      if (DRAW_COORDS) {
        ctx.fillStyle = "#000";
        const text = doubleHeightToString({ x, y });
        const rect = ctx.measureText(text);
        const width = rect.actualBoundingBoxRight - rect.actualBoundingBoxLeft;
        const height =
          rect.actualBoundingBoxDescent - rect.actualBoundingBoxAscent;
        ctx.fillText(text, -width / 2, -height / 2);
      }

      ctx.translate(1.5 * HEX_SIZE, 0.5 * HEX_SIZE * Math.sqrt(3));

      // Don't copy-paste this from above
      if (grid[x + 1] && grid[x + 1][y + 1]) {
        let piece = grid[x + 1][y + 1];
        piece.draw(ctx);
      }
      if (highlight && x + 1 == highlight.x && y + 1 == highlight.y) {
        ctx.strokeStyle = "#0d0";
        ctx.lineWidth = 3;
        ctx.stroke(HEX);
      }
      if (DRAW_COORDS) {
        ctx.fillStyle = "#000";
        const text = doubleHeightToString({ x: x + 1, y: y + 1 });
        const rect = ctx.measureText(text);
        const width = rect.actualBoundingBoxRight - rect.actualBoundingBoxLeft;
        const height =
          rect.actualBoundingBoxDescent - rect.actualBoundingBoxAscent;
        ctx.fillText(text, -width / 2, -height / 2);
      }

      ctx.restore();
    }
  }

  ctx.restore();
}

export default function Grid({ state, handleHexClick }) {
  const [highlight, setHighlight] = React.useState(null);

  const canvasRef = React.useRef(null);

  // The upper-left hex may not be aa ({0, 0}), so determine the offset
  // the hex has from aa, to be added to events (i.e. clicks) later
  let offsetQ = Infinity;
  let offsetR = Infinity;
  // It's also possible the leftmost column's topmost hex is flush with
  // the top of the canvas (meaning a different column has a higher
  // hex); in this case, we'll need to shift pixel calculations
  let leftColLower;
  for (let i = 0; i + 2 < state.length; i += 3) {
    let q = state.charCodeAt(i + 1) - "a".charCodeAt(0);
    let r = state.charCodeAt(i + 2) - "a".charCodeAt(0);

    if (q < offsetQ) {
      leftColLower = r > offsetR;
      offsetQ = q;
    }
    if (r < offsetR) {
      leftColLower = q > offsetQ;
      offsetR = r;
    }
  }

  function handleClick(e) {
    const canvas = canvasRef.current;

    const x = e.clientX - canvas.getBoundingClientRect().left - EDGE_SPACE;
    let y = e.clientY - canvas.getBoundingClientRect().top - EDGE_SPACE;

    if (leftColLower) {
      y += 0.5 * HEX_SIZE * Math.sqrt(3);
    }

    let { q, r } = pixelToAxial({ x, y });
    q += offsetQ;
    r += offsetR;

    handleHexClick({ q, r });
  }

  React.useEffect(() => {
    draw(canvasRef.current, state, highlight);
  }, [state, highlight]);

  return e("canvas", {
    className: "grid",
    onClick: handleClick,
    ref: canvasRef,
  });
}
