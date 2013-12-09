<?hh

function main(bool $x, bool $y) {
  if (!$x && !$y) {
    $z = $x == $y;
    var_dump($z);
  }
}

main(true, true);
