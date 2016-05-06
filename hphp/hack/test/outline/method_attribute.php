<?hh

class C {
  <<A("I'm an attribute and I'm part of the method")>>
  public function methodWithAttribute() {
    // multiline method
    return false;
  }
}
