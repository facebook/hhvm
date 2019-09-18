<?hh

function case1() {
  $x = array();
  var_dump($x[]++);
  var_dump($x);
}

function case3($x) {
  $x[]++;
  var_dump($x);
}
<<__EntryPoint>>
function main_entry(): void {
  case1();
  try {
    case3("asdasd"); // fatal
  } catch(Exception $e) {
    print "\nFatal error: " . $e->getMessage();
  }
}
