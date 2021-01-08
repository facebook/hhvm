<?hh
<<__EntryPoint>>
function main(): void {
  echo "\nNot an array.\n";
  $a = TRUE;
  try {
    foreach ($a as $v) {
      var_dump($v);
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }

  $a = null;
  try {
    foreach ($a as $v) {
      var_dump($v);
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }

  $a = 1;
  try {
    foreach ($a as $v) {
      var_dump($v);
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }

  $a = 1.5;
  try {
    foreach ($a as $v) {
      var_dump($v);
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }

  $a = "hello";
  try {
    foreach ($a as $v) {
      var_dump($v);
    }
  } catch (InvalidForeachArgumentException $e) {
    var_dump($e->getMessage());
  }

  echo "done.\n";
}
