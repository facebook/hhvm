<?hh // experimental

function foo(): void {
  try {
    let catch_me : int = 42;
  } finally {
    echo catch_me; // You cannot catch me
  }
}
