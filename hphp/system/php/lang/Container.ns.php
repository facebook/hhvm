<?hh // partial

namespace HH {

<<__Sealed(KeyedContainer::class)>>
interface Container extends \HH\Rx\Traversable {
}

<<__Sealed(\ConstVector::class, \ConstMap::class, \ConstSet::class, dict::class, keyset::class, vec::class)>>
interface KeyedContainer extends Container, \HH\Rx\KeyedTraversable {
}

}
