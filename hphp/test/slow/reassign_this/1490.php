<?hh

class Foo {
  function Bar() {
    $__this = $this;
    $this = null;
    debug_backtrace();
    $this = $__this;
  }
 }

<<__EntryPoint>> function main(): void {}
