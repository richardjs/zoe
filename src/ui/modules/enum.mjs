class Enum {
    constructor(value) {
        this.value = value;
    }

    toString() {
        return this.value;
    }
}


export class Player extends Enum {
    static P1 = new Player('P1');
    static P2 = new Player('P2');
}


export class Type extends Enum{
    static Ant = new Type('ant');
    static Beetle = new Type('beetle');
    static Grasshopper = new Type('grasshopper');
    static QueenBee = new Type('queen_bee');
    static Spider = new Type('spider');

}
