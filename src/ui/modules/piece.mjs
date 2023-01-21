import { Colors } from './colors.mjs';
import { Player, Type } from './enum.mjs';
import { HEX } from './hex.mjs';


const GAP_SCALE = .95;
const TYPE_SCALE = .85;


export class Piece {
    constructor(player, type) {
        this.player = player;
        this.type = type;
    }

    static fromChar(c) {
        let player = (c === c.toUpperCase()) ? Player.P1 : Player.P2;

        let type = {
            'a': Type.Ant,
            'b': Type.Beetle,
            'g': Type.Grasshopper,
            'q': Type.QueenBee,
            's': Type.Spider
        }[c.toLowerCase()];

        return new Piece(player, type);
    }

    render(ctx) {
        ctx.save()

        ctx.scale(GAP_SCALE, GAP_SCALE);

        ctx.fillStyle = Colors[this.player];
        ctx.fill(HEX);

        ctx.scale(TYPE_SCALE, TYPE_SCALE);

        ctx.fillStyle = Colors[this.type];
        ctx.fill(HEX);

        ctx.restore()
    }
}
