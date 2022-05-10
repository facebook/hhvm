<?hh // partial

namespace HH {

<<__Sealed(KeyedContainer::class)>>
interface Container<+Tv> extends Traversable<Tv> {
}

<<__Sealed(
  /* HH_FIXME[2049] */
  \ConstVector::class,
  /* HH_FIXME[2049] */
  \ConstMap::class,
  /* HH_FIXME[2049] */
  \ConstSet::class,
  AnyArray::class,
)>>
interface KeyedContainer<+Tk as arraykey, +Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {
}

}
