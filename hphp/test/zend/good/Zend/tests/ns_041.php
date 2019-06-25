<?hh
namespace test\ns1;

const FOO = "ok\n";
<<__EntryPoint>> function main(): void {
echo(FOO);
echo(\test\ns1\FOO);
echo(\test\ns1\FOO);
}
