<?hh

function test(): string {
  $f = (): int ==> {
    $x = 42;
    return $x;
//         ^ enforcement-at-caret
  };
  return "hello";
}
