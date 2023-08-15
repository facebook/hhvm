<?hh
namespace test\ns1;

const INI_CONFIG = 0;

function foo($x = \INI_CONFIG) :mixed{
    \var_dump($x);
}
<<__EntryPoint>> function main(): void {
foo();
}
