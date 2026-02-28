<?hh
namespace test\ns1;

function foo() :mixed{
  echo __FUNCTION__,"\n";
}

function bar() :mixed{
  echo __FUNCTION__,"\n";
}

use test\ns1 as ns2;
use test as ns3;
<<__EntryPoint>> function main(): void {

foo();
bar();
\test\ns1\foo();
\test\ns1\bar();
ns2\foo();
ns2\bar();
ns3\ns1\foo();
ns3\ns1\bar();
}
