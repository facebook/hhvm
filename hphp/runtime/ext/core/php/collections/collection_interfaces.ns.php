<?hh
<<file:__EnableUnstableFeatures('readonly')>>

namespace {

<<__Sealed(\HH\Collection::class, ConstMap::class, ConstSet::class, ConstVector::class)>>
interface ConstCollection<+Te> extends Countable, IPureStringishObject  {
  public readonly function isEmpty()[]: bool;
  public readonly function count()[]: int;
  public function items()[]: HH\Iterable<Te>;
}

<<__Sealed(\HH\Collection::class)>>
interface OutputCollection<-Te> {
  public function add(Te $e)[write_props]: this;
  public function addAll(?Traversable<Te> $iterable)[write_props]: this;
}

<<__Sealed(ConstMapAccess::class, SetAccess::class, ConstSet::class)>>
interface ConstSetAccess<+Tm as arraykey> {
  public function contains(arraykey $m)[]: bool;
}

<<__Sealed(MapAccess::class, MutableSet::class)>>
interface SetAccess<Tm as arraykey> extends ConstSetAccess<Tm> {
  public function remove(Tm $m)[write_props]: this;
}

<<__Sealed(ConstMapAccess::class, IndexAccess::class, ConstVector::class)>>
interface ConstIndexAccess<Tk, +Tv> {
  public function at(Tk $k)[]: Tv;
  public function get(Tk $k)[]: ?Tv;
  public readonly function containsKey(Tk $k)[]: bool;
}

<<__Sealed(MapAccess::class, MutableVector::class)>>
interface IndexAccess<Tk, Tv> extends ConstIndexAccess<Tk, Tv> {
  public function set(Tk $k, Tv $v)[write_props]: this;
  public function setAll(
    ?KeyedTraversable<Tk, Tv> $traversable,
  )[write_props]: this;
  public function removeKey(Tk $k)[write_props]: this;
}

<<__Sealed(ConstMap::class, MapAccess::class)>>
interface ConstMapAccess<Tk as arraykey, +Tv>
  extends ConstSetAccess<Tk>, ConstIndexAccess<Tk, Tv> {
}

<<__Sealed(MutableMap::class)>>
interface MapAccess<Tk as arraykey, Tv>
  extends ConstMapAccess<Tk, Tv>, SetAccess<Tk>, IndexAccess<Tk, Tv> {
}

<<__Sealed(
  /* HH_FIXME[2049] */
  ImmVector::class,
  MutableVector::class,
  /* HH_FIXME[2049] */
  Pair::class,
)>>
interface ConstVector<+Tv>
  extends \HH\KeyedContainer<int, Tv>,
          ConstCollection<Tv>,
          ConstIndexAccess<int, Tv>,
          \HH\KeyedIterable<int, Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:ConstVector`

  public function toVArray()[]: varray<Tv>;

}

<<__Sealed(/* HH_FIXME[2049] */ Vector::class)>>
interface MutableVector<Tv>
  extends ConstVector<Tv>,
          \HH\Collection<Tv>,
          IndexAccess<int, Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:MutableVector`
}

<<__Sealed(
  /* HH_FIXME[2049] */
  ImmMap::class,
  MutableMap::class,
)>>
interface ConstMap<Tk as arraykey, +Tv>
  extends \HH\KeyedContainer<Tk, Tv>,
          /* HH_FIXME[2049] */
          ConstCollection<Pair<Tk, Tv>>,
          ConstMapAccess<Tk, Tv>,
          \HH\KeyedIterable<Tk, Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:ConstMap`
}

<<__Sealed(/* HH_FIXME[2049] */ Map::class)>>
interface MutableMap<Tk as arraykey, Tv>
  extends ConstMap<Tk, Tv>,
          /* HH_FIXME[2049] */
          \HH\Collection<Pair<Tk, Tv>>,
          MapAccess<Tk, Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:MutableMap`
}

<<__Sealed(
  /* HH_FIXME[2049] */
  ImmSet::class,
  MutableSet::class,
)>>
interface ConstSet<+Tv as arraykey>
  extends \HH\KeyedContainer<Tv, Tv>,
          ConstCollection<Tv>,
          ConstSetAccess<Tv>,
          \HH\KeyedIterable<arraykey, Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:ConstSet`
}

<<__Sealed(/* HH_FIXME[2049] */ Set::class)>>
interface MutableSet<Tv as arraykey>
 extends ConstSet<Tv>,
         \HH\Collection<Tv>,
         SetAccess<Tv> {
  // TODO(T121423772) This interface should get the methods seen in
  // `hphp/hack/hhi/collections/interfaces.hhi:MutableSet`
}

}

namespace HH {

<<__Sealed(\MutableMap::class, \MutableSet::class, \MutableVector::class)>>
interface Collection<Te> extends \ConstCollection<Te>,
                                 \OutputCollection<Te> {
  public function clear()[write_props]: this;
}

}
