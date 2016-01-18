<?hh

function main(): void {
  $m = Map {
    0 => 'one',
    1 => 'two',
    2 => 'three',
    3 => 'four',
    4 => 'five',
  };

  echo "skipWhile false\n";
  $m2 = $m->skipWhile(function ($v) {
    var_dump($v);
    return false;
  });
  var_dump($m2);

  echo "\n";

  echo "skipWhile true\n";
  $m2 = $m->skipWhile(function ($v) {
    var_dump($v);
    return true;
  });
  var_dump($m2);

  echo "\n";

  echo "takeWhile false\n";
  $m2 = $m->takeWhile(function($v) {
    var_dump($v);
    return false;
  });
  var_dump($m2);

  echo "\n";

  echo "takeWhile true\n";
  $m2 = $m->takeWhile(function($v) {
    var_dump($v);
    return true;
  });
  var_dump($m2);
}

main();
