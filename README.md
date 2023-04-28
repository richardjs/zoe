<img src="https://github.com/richardjs/zoe/raw/master/img/header.jpg" />

# Zoé: Hive AI

Zoé is an engine for playing the game [Hive](https://gen42.com/games/hive).

Zoé uses a [Monte Carlo tree search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search)
algorithm with biased simulations across parallel searches.

## Playing

This repository includes a prototype UI that provides a bare-bones way to play
against Zoé. The easiest way to use it is with Docker. From the repo directory:

```
docker build -t zoe .
docker run -it -p 8000:8000 zoe
```

The UI should then be accessible at
[http://localhost:8000](http://localhost:8000).

At present, the UI is hard-coded to have Zoé play black, and there is no
indication that the algorithm is thinking (outside of the networking tab of the
developer tools; look for a call to `/state/[state]/think`).

(UI improvements are on the [roadmap](https://github.com/richardjs/zoe#roadmap)!
Implementing UHP should also help, as that enables use with
[MzingaViewer](https://github.com/jonthysell/Mzinga/wiki/MzingaViewer).)

The UI is also hard-coded to run 4 workers with 10K iterations each. This
provides a light amount of playing strength with an acceptable move time, but
experients have shown playing strength improves steeply through at least 40K
iterations, and likely continues to improve with higher iteration and worker
counts. This improvement come at the cost of slower move times. See this graph
of Glicko ratings vs. iterations for different worker counts:

<img src="https://github.com/richardjs/zoe/raw/master/img/v1.0-glicko.png" />

To adjust the worker and iteration count, edit the constants at the top of
[src/server.py](https://github.com/richardjs/zoe/blob/master/src/server.py) and
rebuild (and restart) the Docker image.


## Roadmap

- [ ] Implement [UHP](https://github.com/jonthysell/Mzinga/wiki/UniversalHiveProtocol)
- [x] Calculate perft results and [compare with Mzinga](https://github.com/jonthysell/Mzinga/wiki/Perft)
  - Compares favorably through at least depth 7. Next step is to script the
    perft calculations and publish results.
- [ ] Refactor code and improve documentation
- [ ] Implement [MCTS-Solver](https://dke.maastrichtuniversity.nl/m.winands/documents/uctloa.pdf)
- [ ] Add expansion pieces
- [ ] Improve UI (perhaps less of a priority with [MzingaViewer](https://github.com/jonthysell/Mzinga/wiki/MzingaViewer))
- [ ] Improve play!

Zoé was created in 2023 by [Richard Schneider](https://schneiderbox.net).
