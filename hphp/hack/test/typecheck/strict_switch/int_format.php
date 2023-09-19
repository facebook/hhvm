<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
function dec_0_first(int $x): void {
  switch ($x) {
    case 0:
      return;
    case 0b1:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function dec_7_first(int $x): void {
  switch ($x) {
    case 7:
      return;
    case 0xa:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function dec_709_13_8457_first(int $x): void {
  switch ($x) {
    case 709_13_8457:
      return;
    case 050:
      return;
    case 7091_384_57:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function bin_first(int $x): void {
  switch ($x) {
    case 0b0:
      return;
    case 0:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function oct_first(int $x): void {
  switch ($x) {
    case 00:
      return;
    case 0b10:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function oct_first_0762534(int $x): void {
  switch ($x) {
    case 07_625_34:
      return;
    case 0xAE:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function hex_first(int $x): void {
  switch ($x) {
    case 0xBC:
      return;
    case 0b110:
      return;
    default:
      return;
  }
}
