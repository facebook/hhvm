<?hh // experimental

function foo(int $count): void {
  let twice : string = "2x";
  for ($i = 0; $i < $count; $i++) {
    let twice : int = $i * 2;
  }
  echo twice;
}
