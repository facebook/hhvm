<?hh // strict

// According to hack-langspec, the curly braces are mandatory.
// We don't want extra Blocks to be parsed

function foo(): void {
  using ($foo = new Bar()) {
    func();
  }
}

function foo2(): void {
  using ($foo = new Bar()) {
  }
}

function bar(): void {
  using $foo = new Bar();
}
