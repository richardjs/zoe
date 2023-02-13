import { HEX_SIZE } from "./hex.js";

// https://www.redblobgames.com/grids/hexagons/#rounding
function roundCoords({ q, r }) {
  const s = -q - r;

  let qi = Math.round(q);
  let ri = Math.round(r);
  let si = Math.round(s);

  const qd = Math.abs(qi - q);
  const rd = Math.abs(ri - r);
  const sd = Math.abs(si - s);

  if (qd > rd && qd > sd) {
    qi = -ri - si;
  } else if (rd > sd) {
    ri = -qi - si;
  }

  return { q: qi, r: ri };
}

export function axialToDoubleHeight({ q, r }) {
  const x = q;
  const y = 2 * r + q;
  return { x, y };
}

export function axialToString({ q, r }) {
  return (
    String.fromCharCode("a".charCodeAt(0) + q) +
    String.fromCharCode("a".charCodeAt(0) + r)
  );
}

export function pixelToAxial({ x, y }) {
  // The origin for the following calculation should be the center of
  // the upper-leftmost hex, not the upper-left corner of the canvas
  x -= HEX_SIZE;
  y -= 0.5 * HEX_SIZE * Math.sqrt(3);

  // https://www.redblobgames.com/grids/hexagons/#pixel-to-hex
  const q = ((2 / 3) * x) / HEX_SIZE;
  const r = ((-1 / 3) * x + (Math.sqrt(3) / 3) * y) / HEX_SIZE;
  return roundCoords({ q, r });
}
