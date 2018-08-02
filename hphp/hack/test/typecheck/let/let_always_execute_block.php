<?hh // experimental

function foo(): void {
  let x : int = 42;
  {
    let y : int = -1;
    echo x; // 42
    echo y; // -1
  }
  echo x; // 42
  echo y; // error
}
