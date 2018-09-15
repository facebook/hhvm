<?hh

class C {
  public function __call($name, inout $arguments) {
    $arguments = array();
    return null;
  }
}
