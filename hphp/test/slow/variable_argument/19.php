<?hh

class A {
  public function __construct($a, ...$more_args) {
    $args = array_merge(varray[$a], $more_args);
    var_dump(count($args));
    var_dump($args);
  }
}

 <<__EntryPoint>>
function main_19() {
$obj = new A(1, 2, 3);
 $obj = new A('test');
}
