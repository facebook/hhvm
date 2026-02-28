<?hh

class A {
  public function test($a, ...$more_args) :mixed{
    $args = array_merge(vec[$a], $more_args);
    var_dump(count($args));
    var_dump($args);
  }
}

 <<__EntryPoint>>
function main_18() :mixed{
$obj = new A();
 $obj->test('test');
 $obj->test(1, 2, 3);
}
