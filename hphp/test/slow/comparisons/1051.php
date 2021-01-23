<?hh

function foo($p) {
  if ($p) {
    $a = 'foo';
  }
  try {
    if ('' < $a) {
      echo 'yes';
    } else {
      echo 'no';
    }
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    if ($a > '') {
      echo 'yes';
    } else {
      echo 'no';
    }
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1051() {
  foo(false);
}
