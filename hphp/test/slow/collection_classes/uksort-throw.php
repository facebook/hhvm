<?hh

class Ref { public function __construct(public $v)[] {} }

<<__EntryPoint>>
function main() :mixed{
  $m = Map{'c' => 1, 'b' => 2, 'a' => 3};
  $calls = new Ref(0);
  try {
    uksort(inout $m, ($a, $b) ==> {
      echo "in callback\n";
      if (++$calls->v == 2) throw new Exception("lol");
      return $a <=> $b;
    });
  } catch (Exception $e) {
    echo "caught " . $e->getMessage() . "\n";
  }
  var_dump($m);
}
