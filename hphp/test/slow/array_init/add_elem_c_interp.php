<?hh
class X {
  public $foo;

  function test() :mixed{
    $a = dict[
      'null' => $this->foo,
    ];
    return $a;
  }
}

<<__EntryPoint>>
function main_add_elem_c_interp() :mixed{
;

$x = new X;
var_dump($x->test());
}
