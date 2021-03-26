<?hh // partial

namespace HH\Rx {

interface IteratorAggregate extends namespace\Traversable, \IteratorAggregate {
  public function getIterator()[];
}

interface Iterable extends namespace\IteratorAggregate, \HH\Iterable {}

}
