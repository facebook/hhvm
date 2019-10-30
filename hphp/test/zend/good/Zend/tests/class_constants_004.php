<?hh

namespace foo;

const foo = 1;

class foo {
    const foo = 2;
}

interface Ifoo {
    const foo = 4;
}
<<__EntryPoint>> function main(): void {
$const  = __NAMESPACE__ .'\\foo';  // class
$const2 = __NAMESPACE__ .'\\Ifoo'; // interface

\var_dump(
            \foo\foo,
            namespace\foo,
            \foo\foo::foo,
            $const::foo,
            Ifoo::foo,
            $const2::foo
            );
}
