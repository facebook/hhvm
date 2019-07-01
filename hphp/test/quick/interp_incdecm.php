<?hh

function case1() {
  $x = array();
  var_dump($x[]++);
  var_dump($x);
}
case1();

function case3($x) {
  $x[]++;
  var_dump($x);
}
try {
  case3("asdasd"); // fatal
} catch(Exception $e) {
  print "\nFatal error: " . $e->getMessage();
}
