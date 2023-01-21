import { Colors } from './colors.mjs';
import { HEX } from './hex.mjs';


const GAP_SCALE = .95;
const TYPE_SCALE = .85;


export class Piece {
    constructor(player, type) {
        this.player = player;
        this.type = type;
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
