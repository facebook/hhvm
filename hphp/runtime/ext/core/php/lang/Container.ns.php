<?hh

namespace HH {

<<__Sealed(KeyedContainer::class)>>
interface Container<<<__NoAutoBound>> +Tv> extends Traversable<Tv> {
}

<<__Sealed(
  \ConstVector::class,
  \ConstMap::class,
  \ConstSet::class,
  AnyArray::class,
)>>
interface KeyedContainer<<<__NoAutoBound>> +Tk as arraykey, <<__NoAutoBound>> +Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {
}

}
