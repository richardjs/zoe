import { Colors } from "./colors.js";
import { Player, Type } from "./enum.js";
import { HEX } from "./hex.js";

const GAP_SCALE = 0.95;
const TYPE_SCALE = 0.7;

// Applied with GAP_SCALE but not TYPE_SCALE
const ON_TOP_SCALE = TYPE_SCALE;
const ON_TOP_ROTATION = Math.PI / 12;

export class Piece {
  constructor(player, type) {
    this.player = player;
    this.type = type;

    this.on_top = null;
  }

  static fromChar(c) {
    let player = c === c.toUpperCase() ? Player.P1 : Player.P2;

    let type = {
      a: Type.Ant,
      b: Type.Beetle,
      g: Type.Grasshopper,
      q: Type.QueenBee,
      s: Type.Spider,
    }[c.toLowerCase()];

    return new Piece(player, type);
  }

  draw(ctx) {
    ctx.save();
    ctx.scale(GAP_SCALE, GAP_SCALE);

    ctx.fillStyle = Colors[this.player];
    ctx.fill(HEX);

    ctx.scale(TYPE_SCALE, TYPE_SCALE);

    ctx.fillStyle = Colors[this.type];
    ctx.fill(HEX);

    ctx.restore();

    if (this.on_top) {
      ctx.save();

      ctx.scale(ON_TOP_SCALE, ON_TOP_SCALE);
      ctx.rotate(ON_TOP_ROTATION);
      this.on_top.draw(ctx, true);

      ctx.restore();
    }
  }
}
