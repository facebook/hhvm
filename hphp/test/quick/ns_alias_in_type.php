<?hh
namespace A;
use D;
interface I {
  public function foo(D $param):mixed;
}

namespace B;
use D;
use A\I;
class E implements I {
  public function foo(D $param) :mixed{}
}
<<__EntryPoint>> function main(): void {
echo "ok";
}
