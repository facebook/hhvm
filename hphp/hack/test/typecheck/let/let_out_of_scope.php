<?hh // experimental

function foo(): void {
  if (true) {
    let x : int = 42;
  }
  echo x; // fail due to out of scope
}
