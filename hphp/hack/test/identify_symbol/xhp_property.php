<?hh

class C {
  attribute
    string xhp_string = "aaa" @required;

  public function test() {
    $this->:xhp_string;
  }
}
