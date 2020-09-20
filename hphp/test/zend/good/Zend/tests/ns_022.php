<?hh
namespace a\b\c;

use a\b\c as test;

function foo() {
    echo __FUNCTION__,"\n";
}
<<__EntryPoint>> function main(): void {
require "ns_022.inc";
test\foo();
\test::foo();
}
