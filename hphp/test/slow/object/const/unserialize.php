<?hh

<<__Const>>
class C {
  public function __construct(public int $i)[] {}
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C(1);
  var_dump($c);
  $c2 = unserialize(serialize($c));
  var_dump($c2);
  try {
    $c2->i++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($c2);
}
