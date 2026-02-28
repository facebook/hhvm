<?hh

class Blah {
  public function foo() :mixed{ echo "sup\n"; }
}

function main(?Blah $x = null) :mixed{
  if (!$x) {
    $k = vec[1,2,3,4];
  } else {
    $k = null;
  }

  if (is_array($k)) {
    echo count($k) . "\n";
  }
  if (is_object($x)) {
    var_dump($x is Blah);
  } else {
    var_dump(is_null($x));
  }
}


<<__EntryPoint>>
function main_jmp_type_003() :mixed{
main(new Blah);
main();
echo "done\n";
}
