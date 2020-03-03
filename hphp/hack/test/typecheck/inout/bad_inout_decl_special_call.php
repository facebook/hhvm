<?hh // partial

class C {
  public function __call($name, inout $arguments) {
    $arguments = varray[];
    return null;
  }
}
