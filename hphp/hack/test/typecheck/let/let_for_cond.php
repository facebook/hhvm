<?hh // experimental

function foo(): void {
  let count = 10;
  for ($i = 0; $i < count; $i++) {
    let twice : int = $i * 2;
  }
}
