<?hh

namespace HH {

<<__Sealed(KeyedContainer::class)>>
interface Container<+Tv> extends Traversable<Tv> {
}

<<__Sealed(
  \ConstVector::class,
  \ConstMap::class,
  \ConstSet::class,
  AnyArray::class,
)>>
interface KeyedContainer<+Tk as arraykey, +Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {
}

}
