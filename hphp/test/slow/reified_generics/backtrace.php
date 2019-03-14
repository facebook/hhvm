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
    $this->g<T1>();
  }
}

$c = new C<int, shape('a' => int, 'b' => string)>();
$c->f<string>();
