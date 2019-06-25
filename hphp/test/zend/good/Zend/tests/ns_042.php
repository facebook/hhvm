<?hh
namespace test\ns1;

const FOO = "ok\n";

use test\ns1 as ns2;
use test as ns3;
<<__EntryPoint>> function main(): void {
echo FOO;
echo \test\ns1\FOO;
echo \test\ns1\FOO;
echo ns2\FOO;
echo ns3\ns1\FOO;
}
