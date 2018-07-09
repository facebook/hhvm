<?php

namespace HH\Rx {

interface KeyedIterable extends
  namespace\KeyedTraversable,
  namespace\Iterable,
  \HH\KeyedIterable {
  <<__Rx, __OnlyRxIfArgs, __MutableReturn, __MaybeMutable>>
  public function mapWithKey(<<__OnlyRxIfRxFunc>> $callback);
  <<__Rx, __OnlyRxIfArgs, __MutableReturn, __MaybeMutable>>
  public function filterWithKey(<<__OnlyRxIfRxFunc>> $callback);
  <<__Rx, __MutableReturn, __MaybeMutable>>
  public function keys();
}

}
