<?hh

<<__Const>>
class C {
  public function __construct(public int $i)[] {}
  public function __clone(): void {
    $this->i++;
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C(1);
  var_dump($c);
  $d = clone $c;
  var_dump($d);
  try {
    $d->i++;
    echo "FAIL: wrote to const property on cloned object\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($d);
}
