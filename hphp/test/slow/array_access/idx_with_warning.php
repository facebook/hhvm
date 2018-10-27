<?hh

class A implements ArrayAccess {
  public function offsetExists($_) {
    echo "offsetExist\n";
    return true;
  }
  public function offsetGet($_) {
    echo "offsetGet\n";
    return 0;
  }
  public function offsetSet($_, $_) {}
  public function offsetUnset($_) {}
}

<<__EntryPoint>>
function main_idx_with_warning() {
  idx(new A(), 17);
}
