<?hh

class Foo {
  private $blah = vec[1,2,3];

  public function closure_private_prop_access() :mixed{
    $cl = $x ==> $this->blah = $x;
    $cl("asd");
    var_dump(is_string($this->blah));
  }
}

class Bar {
  private $heh = vec[1,2,3];

  public function closure_private_prop_access2() :mixed{
    $cl = () ==> $this->heh = vec[1,2,3];
    $cl();
    var_dump(is_array($this->heh));
  }
}


<<__EntryPoint>>
function main_private_props_007() :mixed{
(new Foo())->closure_private_prop_access();
(new Bar())->closure_private_prop_access2();
}
