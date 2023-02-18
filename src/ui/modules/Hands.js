import { Player, Type } from "./enum.js";
import { HEX, HEX_SIZE } from "./hex.js";
import { Piece } from "./piece.js";
import { e } from "./shortcuts.js";

const NUM_PIECES = {
  [Type.QueenBee]: 1,
  [Type.Ant]: 3,
  [Type.Beetle]: 2,
  [Type.Grasshopper]: 3,
  [Type.Spider]: 2,
};

function draw(canvas, hands) {
  const numP1Types = Object.values(hands[Player.P1]).filter(
    (n) => n > 0
  ).length;
  const numP2Types = Object.values(hands[Player.P2]).filter(
    (n) => n > 0
  ).length;

  canvas.width = HEX_SIZE * 2;
  canvas.height = HEX_SIZE * Math.sqrt(3) * (numP1Types + numP2Types);

  const ctx = canvas.getContext("2d");

  ctx.translate(canvas.width / 2, (-HEX_SIZE * Math.sqrt(3)) / 2);
  for (const player in hands) {
    for (const type in hands[player]) {
      const num = hands[player][type];
      if (num == 0) continue;

      ctx.translate(0, HEX_SIZE * Math.sqrt(3));

      const piece = new Piece(player, type);
      for (let i = 0; i < num - 1; i++) {
        let p = piece;
        while (p.on_top) {
          p = p.on_top;
        }
        p.on_top = new Piece(player, type);
      }
      piece.draw(ctx);
    }
  }
}

export default function Hands({ state }) {
  const canvasRef = React.useRef(null);

  let hands = {
    [Player.P1]: { ...NUM_PIECES },
    [Player.P2]: { ...NUM_PIECES },
  };
  for (let i = 0; i + 2 < state.length; i += 3) {
    const piece = Piece.fromChar(state[i]);
    hands[piece.player][piece.type]--;
  }

  React.useEffect(() => {
    draw(canvasRef.current, hands);
  }, [state]);

  return e("canvas", {
    className: "hands",
    //onClick: handleClick,
    ref: canvasRef,
  });
}
