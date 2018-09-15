<?hh // strict

// According to hack-langspec, the curly braces are mandatory.
// We don't want extra Blocks to be parsed

function foo(): void {
  try {
    func();
  } catch (SomeException $e) {
    func();
  } catch (Exception $e) {
    func();
  } finally {
    func();
  }
}

function foo2(): void {
  try {
  } catch (SomeException $e) {
  } catch (Exception $e) {
  } finally {
  }
}

function bar(): void {
  try {
    func();
  } finally {
    func();
  }
}

function bar2(): void {
  try {
  } finally {
  }
}

function baz(): void {
  try {
    func();
  } catch (Exception $e) {
    func();
  }
}

function baz2(): void {
  try {
  } catch (Exception $e) {
  }
}
