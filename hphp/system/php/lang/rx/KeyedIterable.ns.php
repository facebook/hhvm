<?php

namespace HH\Rx {

interface KeyedIterable extends
  namespace\KeyedTraversable,
  namespace\Iterable,
  \HH\KeyedIterable {
  <<__Rx, __OnlyRxIfArgs, __MutableReturn>>
  public function mapWithKey(<<__OnlyRxIfRxFunc>> $callback);
  <<__Rx, __OnlyRxIfArgs, __MutableReturn>>
  public function filterWithKey(<<__OnlyRxIfRxFunc>> $callback);
  <<__Rx, __MutableReturn>>
  public function keys();
}

}
