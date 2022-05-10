<?hh // partial

namespace HH {

<<__Sealed(
  \HH\Container::class,
  /* HH_FIXME[2049] */
  \DOMNodeList::class,
  /* HH_FIXME[2049] */
  \Imagick::class,
  /* HH_FIXME[2049] */
  \HH\Iterable::class,
  /* HH_FIXME[2049] */
  \HH\Iterator::class,
  /* HH_FIXME[2049] */
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
  /* HH_FIXME[2049] */
  \DOMNamedNodeMap::class,
  /* HH_FIXME[2049] */
  \ImagickPixelIterator::class,
  /* HH_FIXME[2049] */
  \IntlBreakIterator::class,
  /* HH_FIXME[2049] */
  \HH\KeyedIterable::class,
  /* HH_FIXME[2049] */
  \HH\KeyedIterator::class,
  /* HH_FIXME[2049] */
  \MysqlRow::class,
)>>
interface KeyedTraversable<+Tk, +Tv> extends Traversable<Tv> {
}

}
