<?hh

<<__Const>>
class C {
  public function __construct(public int $i)[] {}
}

<<__EntryPoint>>
function test() :mixed{
  // unserializing non-declared prop in a const class throws
  try {
    unserialize('O:1:"C":2:{s:1:"i";i:1;s:1:"j";i:2;}');
    echo "FAIL: unserialized dynamic property in const class\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
