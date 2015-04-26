<?hh

class FooException extends Exception {}

function f() {
  try {
  } catch (FooException $e) {
  } catch (Exception $_) {
    echo $e;
  }
}
