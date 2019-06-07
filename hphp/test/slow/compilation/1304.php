<?hh

class C {
  function foo($a) {
    var_dump($this + $a);
    var_dump($this - $a);
    var_dump($this * $a);
    var_dump($this / $a);
    var_dump($a + $this);
    var_dump($a - $this);
    var_dump($a * $this);
    var_dump($a / $this);
  }
}

<<__EntryPoint>>
function main_1304() {
$obj = new C;
$obj->foo(1);
}
