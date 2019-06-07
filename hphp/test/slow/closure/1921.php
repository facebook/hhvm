<?hh

function foo() {
  $var = 123;
  $abc = 789;
  $a = function () use ($var) {
    var_dump($abc, $var);
    $abc = $var = 333;
  }
;
  var_dump($a());
  var_dump($abc, $var);
}

<<__EntryPoint>>
function main_1921() {
foo();
}
