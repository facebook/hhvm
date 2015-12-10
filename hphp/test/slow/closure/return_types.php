<?hh

function good() {
  $x = 1;

  $f_hack = function (): int use ($x) {
    return $x + 1;
  };

  $f_php7 = function () use ($x): int {
    return $x + 1;
  };

  var_dump($f_hack() + $f_php7());
}

good();

set_error_handler(function ($no, $str) {
  throw new Exception($str);
});

function bad() {
  $x = 1;

  $f_hack = function (): string use ($x) {
    return $x + 1;
  };

  $f_php7 = function () use ($x): string {
    return $x + 1;
  };

  try {
    $f_hack();
    echo "Should have thrown!\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    $f_php7();
    echo "Should have thrown!\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

bad();
