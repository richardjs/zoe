import { Player, Type } from './enum.mjs';
import { HEX_SIZE } from './hex.mjs';
import { Piece } from './piece.mjs';


export class Grid {
    render(ctx) {
        for (let x = 0; x < 10; x+=2) {
            for (let y = 0; y < 10; y += 2) {
                ctx.save();

                ctx.translate(
                   HEX_SIZE + x*1.5*HEX_SIZE,
                   .5*HEX_SIZE*Math.sqrt(3) + y*.5*HEX_SIZE*Math.sqrt(3)
                );
                new Piece(Math.random() > .5 ? Player.P1 : Player.P2, Type.Ant).render(ctx);

                ctx.translate(
                    1.5*HEX_SIZE,
                    .5*HEX_SIZE*Math.sqrt(3)
                );
                new Piece(Player.P1, Type.Ant).render(ctx);

                ctx.restore();
            }
        }
    }
}
