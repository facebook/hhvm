<?hh // experimental

function foo(): void {
  let x : int = 42;
  if (true) {
    echo x; // 42
    let x : string = "Forty-two";
    echo x; // Forty-two
  }
}
