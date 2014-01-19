<?hh

class X {
  protected function foo(): Map<string,string>{
 return Map {
}
;
 }
  function fiz(): void {
    $this->fuz('save', $this->foo());
  }
  function fuz($a, $b): void {
    var_dump($a, $b);
  }
}

$x = new X;
$x->fiz();
