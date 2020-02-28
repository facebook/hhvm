<?hh

class A {
  public function test($a, ...$more_args) {
    $args = array_merge(varray[$a], $more_args);
    var_dump(count($args));
    var_dump($args);
  }
}

 <<__EntryPoint>>
function main_18() {
$obj = new A();
 $obj->test('test');
 $obj->test(1, 2, 3);
}
