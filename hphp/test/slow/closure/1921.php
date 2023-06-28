<?hh

function foo() :mixed{
  $var = 123;
  $abc = 789;
  $a = function() use ($var) {
    try {
      var_dump($abc, $var);
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
    $abc = $var = 333;
  };
  var_dump($a());
  var_dump($abc, $var);
}

<<__EntryPoint>>
function main_1921() :mixed{
  foo();
}
