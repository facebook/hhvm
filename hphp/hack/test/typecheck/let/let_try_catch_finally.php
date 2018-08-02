<?hh // experimental

function foo(): void {
  try {
    let catch_me : int = 42;
  } catch (Exception $e) {
    // Do nothing
  } finally {
    echo catch_me; // You cannot catch me
  }
}
