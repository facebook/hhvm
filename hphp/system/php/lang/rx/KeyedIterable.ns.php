<?hh // partial

namespace HH\Rx {

interface KeyedIterable extends
  namespace\KeyedTraversable,
  namespace\Iterable,
  \HH\KeyedIterable {
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey(<<__AtMostRxAsFunc>> $callback);
  <<__Pure, __AtMostRxAsArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__AtMostRxAsFunc>> $callback);
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function keys();
}

}
