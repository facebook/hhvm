<?hh

function f($a) :mixed{
 echo "test$a\n";
 return 1;
 }
function bug2($a, $b) :mixed{
  return isset($b[f($a++)], $b[f($a++)], $b[f($a++)]);
}

<<__EntryPoint>>
function main_1515() :mixed{
bug2(0, vec[]);
}
