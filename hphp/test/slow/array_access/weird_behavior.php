<?hh

function queryMTest($m) {
  var_dump($m[0]);
}

function setMTest($m) {
  $m[2] = 1;
  var_dump($m);
}

<<__EntryPoint>>
function main() {
  $test = [null, false, true, 0, 3, 5.5, "", "abcd"];
  print "---- test query --------\n";
  foreach($test as $t) {
    try {
      queryMTest($t);
    } catch(Exception $e) {
      print $e->getMessage() . "\n";
    }
  }

  print "---- test set --------\n";
  foreach($test as $t) {
    try {
      setMTest($t);
    } catch(Exception $e) {
      print $e->getMessage() . "\n";
    }
  }
}
