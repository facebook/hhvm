<?php

namespace HH {

<<__Sealed(\HH\KeyedContainer::class, \ConstSet::class)>>
interface Container extends \HH\Rx\Traversable {
}

<<__Sealed(\Indexish::class)>>
interface KeyedContainer extends \HH\Container, \HH\Rx\KeyedTraversable {
}

}
