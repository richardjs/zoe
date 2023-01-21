export const HEX_SIZE = 75;

export const HEX = new Path2D();
HEX.moveTo(-HEX_SIZE/2, .5*HEX_SIZE*Math.sqrt(3));
HEX.lineTo(HEX_SIZE/2, .5*HEX_SIZE*Math.sqrt(3));
HEX.lineTo(HEX_SIZE, 0);
HEX.lineTo(HEX_SIZE/2, -.5*HEX_SIZE*Math.sqrt(3));
HEX.lineTo(-HEX_SIZE/2, -.5*HEX_SIZE*Math.sqrt(3));
HEX.lineTo(-HEX_SIZE, 0);
HEX.lineTo(-HEX_SIZE/2, .5*HEX_SIZE*Math.sqrt(3));
