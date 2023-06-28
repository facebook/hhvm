<?hh

class Foo {
  function Bar() :mixed{
    $__this = $this;
    $this = null;
    debug_backtrace();
    $this = $__this;
  }
 }

