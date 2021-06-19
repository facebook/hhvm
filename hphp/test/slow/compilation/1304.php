<?hh

class C {
  function foo($a) {
    try {
      var_dump($this + $a);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this - $a);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this * $a);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this / $a);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a + $this);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a - $this);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a * $this);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a / $this);
    } catch (TypecastException $e) {
      var_dump($e->getMessage());
    }
  }
}

<<__EntryPoint>>
function main_1304() {
$obj = new C;
$obj->foo(1);
}
