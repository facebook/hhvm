<?hh // experimental

function foo(): void {
  try {
    let catch_me : int = 42;
  } catch (Exception $e) {
    echo catch_me; // You cannot catch me
  }
}
