<?hh

function foo() :mixed{ return 1; }

function main() :mixed{
  $x = null;
  $y = $z = foo();
  try {
    $f = $y + $z;  // Add is constprop, and generally a PEI
    $x = "heh";
    throw new Exception('heh');
  } catch (Exception $y) {
    if (is_null($x)) { echo "impossible\n"; }
    var_dump($x);
    echo $x; // should constant propagate "heh" to here
    echo "\n";
  }
  var_dump($f);
}

<<__EntryPoint>>
function main_constprop_exceptions() :mixed{
main();
}
