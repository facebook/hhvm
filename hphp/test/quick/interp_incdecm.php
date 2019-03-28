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
case3(true);   // warning, not an array
case3("asdasd"); // fatal
