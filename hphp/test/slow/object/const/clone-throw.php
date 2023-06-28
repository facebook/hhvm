<?hh

<<__Const>>
class C {
  public static ?C $c = null;

  public function __construct(public int $i)[] {}
  public function __clone(): void {
    $this->i++;

    // stash this and throw
    self::$c = $this;
    throw new Exception('sneaky');
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C(1);
  var_dump($c);
  try {
    $d = clone $c;
  } catch (Exception $_) {}
  $d = C::$c;
  var_dump($d);
  try {
    $d->i++;
    echo "FAIL: wrote to const property on cloned object\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($d);
}
