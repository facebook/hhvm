<?hh

namespace foo;

trait IFoo { }

trait ITest { }

<<__EntryPoint>> function main(): void {
\var_dump(\trait_exists('IFoo'));
\var_dump(\trait_exists('foo\\IFoo'));
\var_dump(\trait_exists('FOO\\ITEST'));
}
