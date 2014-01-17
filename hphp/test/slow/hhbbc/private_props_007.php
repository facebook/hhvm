<?hh

class Foo {
  private $blah = array(1,2,3);

  public function closure_private_prop_access() {
    $cl = $x ==> $this->blah = $x;
    $cl("asd");
    var_dump(is_string($this->blah));
  }
}

class Bar {
  private $heh = array(1,2,3);

  public function closure_private_prop_access2() {
    $cl = () ==> $this->heh = array(1,2,3);
    $cl();
    var_dump(is_array($this->heh));
  }
}

(new Foo())->closure_private_prop_access();
(new Bar())->closure_private_prop_access2();
