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

function draw(canvas, player, hand) {
  const numTypes = Object.values(hand).filter((n) => n > 0).length;

  canvas.width = HEX_SIZE * 2;
  canvas.height = HEX_SIZE * Math.sqrt(3) * numTypes;

  const ctx = canvas.getContext("2d");

  ctx.translate(canvas.width / 2, (-HEX_SIZE * Math.sqrt(3)) / 2);
  for (const type in hand) {
    const num = hand[type];
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

export default function Hand({ state, player, handleActionInput }) {
  const canvasRef = React.useRef(null);

  const turn = state.slice(-1) === "1" ? Player.P1 : Player.P2;
  const initialState = state.length === 1;

  let hand = { ...NUM_PIECES };

  for (let i = 0; i + 2 < state.length; i += 3) {
    const piece = Piece.fromChar(state[i]);
    if (piece.player === player) {
      hand[piece.type]--;
    }
  }

  function handleClick(e) {
    if (player !== turn) return;

    const canvas = canvasRef.current;

    const x = e.clientX - canvas.getBoundingClientRect().left;
    const y = e.clientY - canvas.getBoundingClientRect().top;

    const row = Math.floor(y / (HEX_SIZE * Math.sqrt(3)));

    let i = 0;
    for (let type in hand) {
      if (!hand[type]) continue;

      if (row == i) {
        let pieceChar = {
          [Type.QueenBee]: "q",
          [Type.Ant]: "a",
          [Type.Beetle]: "b",
          [Type.Grasshopper]: "g",
          [Type.Spider]: "s",
        }[type];
        if (!initialState) {
          handleActionInput("+" + pieceChar);
        } else {
          handleActionInput("+" + pieceChar + "aa");
        }
        return;
      }

      i++;
    }
  }

  React.useEffect(() => {
    draw(canvasRef.current, player, hand);
  }, [state]);

  return e("canvas", {
    className: "hand",
    id: "hand-" + player,
    onClick: handleClick,
    ref: canvasRef,
  });
}
