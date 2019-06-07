<?hh

namespace foo;

class bar {
}
<<__EntryPoint>> function main() {
\class_alias('foo\bar', 'foo');

\var_dump(new \foo);
\var_dump(new foo);
}
