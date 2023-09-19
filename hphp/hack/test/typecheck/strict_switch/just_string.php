<?hh

function just_string(string $x): void {
  switch ($x) {
    case "":
      return;
    default:
      return;
  }
}

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

function non_literal(string $x): void {
  $y = "";

  switch ($x) {
    case $y:
      return;
    default:
      return;
  }
}

function missing_default(string $x): void {
  switch ($x) {
    case "":
      return;
  }
}

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
