<?hh
class X {
  public $foo;

  function test() {
    $a = darray[
      'null' => $this->foo,
    ];
    return $a;
  }
}

<<__EntryPoint>>
function main_add_elem_c_interp() {
;

$x = new X;
var_dump($x->test());
}
