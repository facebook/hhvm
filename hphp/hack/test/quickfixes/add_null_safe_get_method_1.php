<?hh

class Thing {
  public method(): void {}
}

function foo(): void {
  if (1 < 2) {
    $thing = new Thing();
  } else {
    $thing = null;
  }
  $thing->method();
}
