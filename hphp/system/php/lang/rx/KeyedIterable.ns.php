<?hh // partial

namespace HH\Rx {

interface KeyedIterable extends
  namespace\KeyedTraversable,
  namespace\Iterable,
  \HH\KeyedIterable {
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey(<<__AtMostRxAsFunc>> $callback);
  <<__Rx, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> $callback);
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function keys();
}

}
