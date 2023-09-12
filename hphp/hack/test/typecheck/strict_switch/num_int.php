<?hh

function just_num(num $x): void {
  switch ($x) {
    case 0:
      return;
    case 0.5:
      return;
    default:
      return;
  }
}

function missing_num(num $x): void {
  switch ($x) {
    case null:
      return;
  }
}

function redundant_num(num $x): void {
  switch ($x) {
    case 0:
      return;
    case true:
      return;
    default:
      return;
  }
}
