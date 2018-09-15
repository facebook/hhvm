<?hh // experimental

function foo(int $x): void {
  let shadow : string = "Shadow";
  while ($x < 10) {
    let shadow : int = 42;
  }
  echo shadow;
  $shadows = shadow."s";
}
