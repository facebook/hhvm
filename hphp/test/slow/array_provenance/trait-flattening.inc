<?hh

trait baz {
  public function frob() {
    return vec[1, 2, 3];
  }

  public function fizaz() {
    return vec(__hhvm_intrinsics\launder_value(varray[1, 2, 3]));
  }
}

