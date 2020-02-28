<?hh

class A {
  public function __call($method, $args) {
    foreach ($args as $a) {
 var_dump($a);
 }
    var_dump(array_pop(inout $args));
    if (isset($args[1])) {
 var_dump($args[1]);
 }
    reset(inout $args);
    if (key($args) === 0) {
       $args = varray[5];
    }
    if (current($args) === 0) {
       $args = varray[5];
    }
    if (next(inout $args) === 0) {
       $args = varray[5];
    }
    try { var_dump($args['1']); }
    catch (Exception $e) { echo $e->getMessage()."\n"; }
    try { var_dump($args['hi']); }
    catch (Exception $e) { echo $e->getMessage()."\n"; }
    $args = $args + darray[2 => 0, 3 => true, 4 => true];
     var_dump($args);
  }
}

<<__EntryPoint>>
function main_240() {
$obj = new A;
$obj->foo(1, 2, 3);
}
