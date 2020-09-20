<?hh // partial

namespace HH\Rx {

interface IteratorAggregate extends namespace\Traversable, \IteratorAggregate {
  <<__Pure, __MutableReturn, __MaybeMutable>>
  public function getIterator();
}

interface Iterable extends namespace\IteratorAggregate, \HH\Iterable {}

}
