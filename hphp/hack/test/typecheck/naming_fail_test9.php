<?hh // partial

class FooException extends Exception {}

function f() {
  try {
    might_throw();
  } catch (FooException $e) {
  } catch (Exception $_) {
    echo $e;
  }
}

function might_throw(): void {}
