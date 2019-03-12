<?hh // partial

namespace HH\Rx {

interface IteratorAggregate extends namespace\Traversable, \IteratorAggregate {
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function getIterator();
}

interface Iterable extends namespace\IteratorAggregate, \HH\Iterable {}

}
