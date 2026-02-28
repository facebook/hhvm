<?hh

namespace HH {

interface KeyedIterable<+Tk, +Tv> extends Iterable<Tv>, KeyedTraversable<Tk, Tv> {
  // TODO(T121800572) We should add in the methods defined in the version of
  // this class in `hphp/hack/hhi/interfaces.hhi`
  // TODO(T125922750) This file *used* to contain a definition for `mapWithKeys`
  // but it has been removed for now to unblock typechecking `Set`.

  public function filterWithKey(
    (function(Tk, Tv)[_]: bool) $callback
  )[ctx $callback]: KeyedIterable<Tk, Tv>;

  public function keys(): Iterable<Tk>;

}

}
