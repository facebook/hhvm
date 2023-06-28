<?hh

class Foo {
  function Bar() :mixed{
    $__this = $this;
    $this = null;
    debug_backtrace();
    $this = $__this;
  }
}
<<__EntryPoint>> function main(): void {
$f = new Foo;

$f->Bar();

echo "OK\n";
}
