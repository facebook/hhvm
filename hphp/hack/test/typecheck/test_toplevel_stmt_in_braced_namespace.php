<?hh
namespace Foo {
  function foo(int $x): void {
    echo $x + $x;
  }
  foo(true);
}
