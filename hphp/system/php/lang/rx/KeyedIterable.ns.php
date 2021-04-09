<?hh // partial

namespace HH\Rx {

interface KeyedIterable extends
  namespace\KeyedTraversable,
  namespace\Iterable,
  \HH\KeyedIterable {
  public function mapWithKey((function()[_]: void) $callback)[ctx $callback];
  public function filterWithKey((function()[_]: void) $callback)[ctx $callback];
  public function keys()[];
}

}
