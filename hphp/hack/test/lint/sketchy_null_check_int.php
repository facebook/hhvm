<?hh

function sketchy_null_check_int(?int $i): void {
  if ($i) {
    echo($i);
  }
}
