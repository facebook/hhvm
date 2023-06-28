<?hh

namespace A;
const CO = "a";
function f() :mixed{ return "a"; }

namespace B;
const CO = "b";
function f() :mixed{ return "b"; }

use A\f;
use A\CO;
<<__EntryPoint>> function main(): void {
\var_dump(f());
\var_dump(CO);
}
