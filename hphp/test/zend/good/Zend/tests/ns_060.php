<?hh
namespace Foo;
use Bar\A as B;
class A {}

function test(): void {
  $a = new B;
  $b = new A;
  echo \get_class($a)."\n";
  echo \get_class($b)."\n";
}

namespace Bar;
use Foo\A as B;

function test(): void {
  $a = new B;
  $b = new A;
  echo \get_class($a)."\n";
  echo \get_class($b)."\n";
}

class A {}

<<__EntryPoint>>
function entrypoint_ns_060(): void {
  \Foo\test();
  \Bar\test();
}
