<?hh

namespace foo;

interface foo {
    const foo = 2;
}

function foo($x = \foo\foo::foo) :mixed{
    \var_dump($x);
}

<<__EntryPoint>> function main(): void {
\error_reporting(\E_ALL);
foo();
}
