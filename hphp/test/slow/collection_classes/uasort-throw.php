<?hh

class Ref { public function __construct(public $v)[] {} }

<<__EntryPoint>>
function main() :mixed{
  $m = Map{1 => 'c', 2 => 'b', 3 => 'a'};
  $calls = new Ref(0);
  try {
    uasort(inout $m, ($a, $b) ==> {
      echo "in callback\n";
      if (++$calls->v == 2) throw new Exception("lol");
      return $a <=> $b;
    });
  } catch (Exception $e) {
    echo "caught " . $e->getMessage() . "\n";
  }
  var_dump($m);
}
