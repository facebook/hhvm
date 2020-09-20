<?hh // partial

namespace HH\Rx {

interface Iterator extends namespace\Traversable, \HH\Iterator {
  <<__Pure, __MaybeMutable>>
  public function current();
  <<__Pure, __MaybeMutable>>
  public function key();
  <<__Pure, __Mutable>>
  public function next();
  <<__Pure, __Mutable>>
  public function rewind();
  <<__Pure, __MaybeMutable>>
  public function valid();
}

}
