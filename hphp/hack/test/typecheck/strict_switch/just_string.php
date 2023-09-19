<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_string(string $x): void {
  switch ($x) {
    case "":
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function redundant_special_string(string $x): void {
  switch ($x) {
    case '"':
      return;
    case "\"":
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function non_literal(string $x): void {
  $y = "";

  switch ($x) {
    case $y:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function missing_default(string $x): void {
  switch ($x) {
    case "":
      return;
  }
}

<<__StrictSwitch>>
function redundant_literal(string $x): void {
  switch ($x) {
    case "":
      return;
    case ".":
      return;
    case "":
      return;
    case ".":
      return;
    default:
      return;
  }
}
