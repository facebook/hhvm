<?hh

<<__Const>>
class C {
  public function __construct(public int $i, public int $j)[] {}
}

<<__EntryPoint>>
function test() :mixed{
  // props are in the wrong order so unserialize will take the slow path
  $c = unserialize('O:1:"C":2:{s:1:"j";i:2;s:1:"i";i:1;}');
  var_dump($c);
  try {
    $c->i++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($c);
}
