<?hh

namespace foo;

interface IFoo { }

interface ITest extends IFoo { }

interface IBar extends IFoo { }

<<__EntryPoint>> function main(): void {
\var_dump(\interface_exists('IFoo'));
\var_dump(\interface_exists('foo\\IFoo'));
\var_dump(\interface_exists('foo\\ITest'));
}
