<?hh

function check1(bool $x): int {
  if ($x == true) {
    return 1;
  }
}
function check2(bool $x): int {
  if ($x == false) {
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

function check5(bool $x): int {
  if ($x === true) {
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
