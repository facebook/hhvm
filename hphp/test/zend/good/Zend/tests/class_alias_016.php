<?hh

namespace foo;

class bar {
}
<<__EntryPoint>> function main(): void {
\class_alias('foo\bar', 'foo');

\var_dump(new \foo);
\var_dump(new foo);
}
