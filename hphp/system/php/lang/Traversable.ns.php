<?hh // partial

namespace HH {

<<__Sealed(
  \DOMNodeList::class,
  \Imagick::class,
  \HH\Iterable::class,
  \HH\Iterator::class,
  \IteratorAggregate::class,
  \HH\KeyedTraversable::class,
  \ResourceBundle::class,
  \SplHeap::class,
  \SplObjectStorage::class,
  \HH\Rx\Traversable::class,
  \SimpleXMLElement::class
)>>
interface Traversable {
}

<<__Sealed(
  \ArrayIterator::class,
  \AsyncMysqlRowBlock::class,
  \DOMNamedNodeMap::class,
  \ImagickPixelIterator::class,
  \IntlBreakIterator::class,
  \HH\KeyedIterable::class,
  \HH\KeyedIterator::class,
  \MysqlRow::class,
  \HH\Rx\KeyedTraversable::class
)>>
interface KeyedTraversable extends \HH\Traversable {
}

}
