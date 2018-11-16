<?hh

class C<reify Ta, reify Tb> {
  function g<reify T1>() {
    try {
      throw new Exception();
    } catch (Exception $e) {
      echo $e->getTraceAsString() . "\n";
    }
  }
  function f<reify T1>() {
    $this->g<reify T1>();
  }
}

$c = new C<reify int, reify shape('a' => int, 'b' => string)>();
$c->f<reify string>();
