<?hh // partial

namespace HH\Rx {

interface Iterator extends namespace\Traversable, \HH\Iterator {
  <<__Rx, __MaybeMutable>>
  public function current();
  <<__Rx, __MaybeMutable>>
  public function key();
  <<__Rx, __Mutable>>
  public function next();
  <<__Rx, __Mutable>>
  public function rewind();
  <<__Rx, __MaybeMutable>>
  public function valid();
}

}
