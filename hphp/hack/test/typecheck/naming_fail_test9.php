<?hh

class FooException extends Exception {}

function f(): void {
  try {
    might_throw();
  } catch (FooException $e) {
  } catch (Exception $_) {
    echo $e;
  }
}

function might_throw(): void {}
