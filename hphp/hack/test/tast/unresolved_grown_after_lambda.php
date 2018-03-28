<?hh // strict

function test(int $i, string $s): int {
  $items = Vector { $i };
  $f = (): int ==> {
    // When this function is typechecked, we have
    //   $item : Tunresolved [int]
    // But in the TAST, we should have
    //   $item : Tunresolved [int; string]
    $item = $items[1];
    return $item;
  };
  $items[] = $s;
  return $f();
}
