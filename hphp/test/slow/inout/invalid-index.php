<?hh

function foo(inout $x) { var_dump($x); $x = 12; }

function checkStatic() {
  $a = darray['apple' => darray['orange' => 5]];
  try { foo(inout $a[10]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { foo(inout $a['apple']['banana']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
  try { foo(inout $a['apple']['orange']); } catch (Exception $e) { echo $e->getMessage()."\n"; }

  try {
    foo(inout $a[50][100]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function getOne(inout $a, $k) {
  try {
    foo(inout $a[$k]);
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

function getTwo(inout $a, $k1, $k2) {
  try {
    foo(inout $a[$k1][$k2]);
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

function checkNonStatic($a) {
  getOne(inout $a, 10);
  getTwo(inout $a, 'apple', 'banana');
  getTwo(inout $a, 'apple', 'orange');
  getTwo(inout $a, 50, 100);
  var_dump($a);
}

function checkNums($a) {
  getTwo(inout $a, 0, 1);
  getTwo(inout $a, 0, 100);
  getOne(inout $a, 0);
  getOne(inout $a, 50);
  getTwo(inout $a, 50, 100);
}

<<__EntryPoint>>
function main() {
  checkStatic();
  $a = darray['apple' => darray['orange' => 5]];
  $b = dict['apple' => darray['orange' => 5]];
  $c = keyset[0, 1, 2];
  $d = vec[vec[0, 1, 2], 3, 4];
  checkNonStatic($a);
  checkNonStatic($b);
  checkNums($c);
  checkNums($d);
}
