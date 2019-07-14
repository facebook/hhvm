<?hh
namespace a\b\c;

use a\b\c as test;

require "ns_022.inc";

function foo() {
    echo __FUNCTION__,"\n";
}
<<__EntryPoint>> function main(): void {
test\foo();
\test::foo();
}
