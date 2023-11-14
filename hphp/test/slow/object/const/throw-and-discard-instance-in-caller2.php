<?hh

<<__NEVER_INLINE>>
function throw_exception() {
  throw new Exception('sneaky');
}

<<__Const>>
class C {
  public int $i = 0;
  public static ?C $c = null;

  public function __construct(bool $throw) {
    // stash this and throw
    self::$c = $this;
    if ($throw) throw_exception();
  }
}

function construct(bool $throw) {
  // We don't keep a copy of the instance here in the caller
  new C($throw);
}

<<__NEVER_INLINE>>
function test($throw): void {
  try {
    construct($throw);
  } catch (Exception $_) {}

  try {
    C::$c->i = 99;
    echo "FAIL: wrote to scalar property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  test(true);
  test(false);
}
