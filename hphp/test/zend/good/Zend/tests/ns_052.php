<?hh
namespace test\ns1;

const INI_ALL = 0;

function foo($x = \INI_ALL) {
    \var_dump($x);
}
<<__EntryPoint>> function main(): void {
foo();
}
