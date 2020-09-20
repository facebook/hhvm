<?hh // partial

namespace HH\Rx {

<<__Sealed(
  namespace\KeyedTraversable::class,
  namespace\Iterator::class,
  namespace\IteratorAggregate::class,
  \HH\Container::class
)>>
interface Traversable extends \HH\Traversable {}

<<__Sealed(
  namespace\KeyedIterable::class,
  namespace\KeyedIterator::class,
  \HH\KeyedContainer::class
)>>
interface KeyedTraversable extends namespace\Traversable, \HH\KeyedTraversable {
}

}
