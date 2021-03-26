<?hh // partial

namespace HH\Rx {

interface Iterator extends namespace\Traversable, \HH\Iterator {
  public function current()[];
  <<__Pure, __MaybeMutable>>
  public function key();
  public function next()[write_props];
  public function rewind()[write_props];
  public function valid()[];
}

}
