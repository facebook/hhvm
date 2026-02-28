<?hh
function plus(int $i): int {
  if ($i == 0) {
    return 1;
  } else if ($i == 1) {
    return 2;
  } else if ($i == 2){
    return 3;
  } else {
    return $i + 1;
  }
}
