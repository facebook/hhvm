<?hh

function test($a,$b = 0) :mixed{
  if ($a == 2) {
    if ($b == 1) {
      return;
    }
    $a = 5;
  }
  if ($a == 3) {
    var_dump($a);
  }
}

<<__EntryPoint>>
function main_1414() :mixed{
test(3);
}
