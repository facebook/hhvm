<?hh

function f($a) {
 echo "test$a\n";
 return 1;
 }
function bug2($a, $b) {
  return isset($b[f($a++)], $b[f($a++)], $b[f($a++)]);
}

<<__EntryPoint>>
function main_1515() {
bug2(0, varray[]);
}
