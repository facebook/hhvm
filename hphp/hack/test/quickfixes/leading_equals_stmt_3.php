<?hh

function foo(): void {
  if (true) {
    =((int $y) ==> {
      $x = $y;
      return $x;
    })() + 1;
  }
}
