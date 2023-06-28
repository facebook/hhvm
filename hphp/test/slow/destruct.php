<?hh

class C {
  function __construct(public $prop) {}
};

<<__EntryPoint>>
function main() :mixed{
  $v = null;
  $d = null;
  $o = null;
  for ($i = 0; $i < 500000; $i++) {
    $v = vec[$v];
    $d = dict["foo" => $d];
    $o = new C($o);
  }
  echo "done\n";
}

