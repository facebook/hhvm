<?hh
function check1(bool $x): int {
  if ($x == true) {
    return 1;
  }
}
function check2(bool $y): int {
  if ($y == false) {
    return 2;
  }
}

function check3(bool $x): int {
  if (true == $x) {
    return 2;
  }
}
function check4(bool $x): int {
  if (false == $x) {
    return 2;
  }
}

function check5(bool $y): int {
  if ($y === true) {
    return 1;
  }
}
function check6(bool $x): int {
  if ($x === false) {
    return 2;
  }
}

function check7(bool $x): int {
  if (true === $x) {
    return 2;
  }
}
function check8(bool $x): int {
  if (false === $x) {
    return 2;
  }
}

function check9(bool $x): int {
  if (($x > 1) === true) {
    return 1;
  }
}

function check10(mixed $x): int {
  if ($x == true) {
    return 1;
  }
}

function check11(bool $x, int $i): int {
  while ($x == true) {
    $i++;
  }
}
function check12(bool $x, int $i): int {
  do {
    $i++;
  } while ($x == false);
}
function check13(bool $x, int $i): int {
  for ($j = 0; $x == true; ++$j) {
    $i++;
  }
}
function check14(bool $x, bool $y, int $i): int {
  $y = $x === true;
}
function check15(bool $x, bool $y, int $i): int {
  foo($x === true);
}
