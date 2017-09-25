<?hh

class Blah {
  public function foo() { echo "sup\n"; }
}

function main(Blah $x = null) {
  if (!$x) {
    $k = array(1,2,3,4);
  } else {
    $k = null;
  }

  if (is_array($k)) {
    echo count($k) . "\n";
  }
  if (is_object($x)) {
    var_dump($x instanceof Blah);
  } else {
    var_dump(is_null($x));
  }
}

main(new Blah);
main();
echo "done\n";
