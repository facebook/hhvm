<?hh

trait baz {
  public function frob() {
    return varray[1, 2, 3];
  }

  public function fizaz() {
    return varray(__hhvm_intrinsics\launder_value(vec[1, 2, 3]));
  }
}

