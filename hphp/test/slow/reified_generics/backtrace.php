<?hh

class C<reified Ta, reified Tb> {
  function g<reified T1>() {
    try {
      throw new Exception();
    } catch (Exception $e) {
      echo $e->getTraceAsString() . "\n";
    }
  }
  function f<reified T1>() {
    $this->g<reified T1>();
  }
}

$c = new C<reified int, reified shape('a' => int, 'b' => string)>();
$c->f<reified string>();
