<?hh
namespace a\b\c;

use a\b\c as Test;

function foo() :mixed{
    echo __FUNCTION__,"\n";
}
<<__EntryPoint>> function main(): void {
require "ns_022.inc";
Test\foo();
\Test::foo();
}
