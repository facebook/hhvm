<?hh // partial

namespace HH {

interface KeyedIterable<+Tk, +Tv> extends Iterable<Tv>, KeyedTraversable<Tk, Tv> {

  public function mapWithKey<Tu>(
    (function(Tk, Tv)[_]: Tu) $callback
  )[ctx $callback]: KeyedIterable<Tk, Tu>;

  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $callback
  )[ctx $callback]: KeyedIterable<Tk, Tv>;

  public function keys(): Iterable<Tk>;

}

}
