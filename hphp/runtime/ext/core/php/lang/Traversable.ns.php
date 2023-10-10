<?hh

namespace HH {

<<__Sealed(
  \HH\Container::class,
  \DOMNodeList::class,
  /* HH_FIXME[2049] */
  \Imagick::class,
  \HH\Iterable::class,
  \HH\Iterator::class,
  \IteratorAggregate::class,
  \HH\KeyedTraversable::class,
  /* HH_FIXME[2049] */
  \ResourceBundle::class,
  /* HH_FIXME[2049] */
  \SplHeap::class,
  /* HH_FIXME[2049] */
  \SimpleXMLElement::class
)>>
interface Traversable<+Tv> {
}

<<__Sealed(
  \HH\KeyedContainer::class,
  /* HH_FIXME[2049] */
  \ArrayIterator::class,
  /* HH_FIXME[2049] */
  \AsyncMysqlRowBlock::class,
  \DOMNamedNodeMap::class,
  /* HH_FIXME[2049] */
  \ImagickPixelIterator::class,
  /* HH_FIXME[2049] */
  \IntlBreakIterator::class,
  \HH\KeyedIterable::class,
  \HH\KeyedIterator::class,
  /* HH_FIXME[2049] */
  \MysqlRow::class,
)>>
interface KeyedTraversable<+Tk, +Tv> extends Traversable<Tv> {
}

}
