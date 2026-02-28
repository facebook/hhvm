<?hh

class C {
  function foo($a) :mixed{
    try {
      var_dump($this + $a);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this - $a);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this * $a);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($this / $a);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a + $this);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a - $this);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a * $this);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
    try {
      var_dump($a / $this);
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
  }
}

<<__EntryPoint>>
function main_1304() :mixed{
$obj = new C;
$obj->foo(1);
}
