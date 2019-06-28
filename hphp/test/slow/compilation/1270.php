<?hh

class A {
  function __call($a, $b) {
    var_dump($a, $b[0], $b[1]);
  }
}

 <<__EntryPoint>>
function main_1270() {
  $obj = new A();
  $a = 1;
  $b = 2;
  $obj->test($a, $b);
}
