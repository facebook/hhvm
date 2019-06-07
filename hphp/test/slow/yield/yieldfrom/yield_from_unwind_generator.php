<?hh

class GenClass {
  function __construct() { echo "Making GenClass\n"; }
  function genInner()    { throw new Exception; yield 5; }
}

function genOuter() {
  $x = (new GenClass)->genInner(); // $x is now a generator containing the only reference to a GenClass
  try {
    yield from $x;
  } catch (Exception $ex) {
    echo "Caught Exception (outer)\n";
  }
  echo "Finished genOuter()\n";
}


<<__EntryPoint>>
function main_yield_from_unwind_generator() {
  $o = genOuter();
  try {
    $o->next();
  } catch (Exception $ex) {
    echo "Caught Exception (main)\n";
  }
}
