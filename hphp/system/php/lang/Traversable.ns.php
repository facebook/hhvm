<?hh // partial

namespace HH {

<<__Sealed(
  \HH\Container::class,
  \DOMNodeList::class,
  \Imagick::class,
  \HH\Iterable::class,
  \HH\Iterator::class,
  \IteratorAggregate::class,
  \HH\KeyedTraversable::class,
  \ResourceBundle::class,
  \SplHeap::class,
  \SimpleXMLElement::class
)>>
interface Traversable {
}

<<__Sealed(
  \HH\KeyedContainer::class,
  \ArrayIterator::class,
  \AsyncMysqlRowBlock::class,
  \DOMNamedNodeMap::class,
  \ImagickPixelIterator::class,
  \IntlBreakIterator::class,
  \HH\KeyedIterable::class,
  \HH\KeyedIterator::class,
  \MysqlRow::class,
)>>
interface KeyedTraversable extends \HH\Traversable {
}

}
