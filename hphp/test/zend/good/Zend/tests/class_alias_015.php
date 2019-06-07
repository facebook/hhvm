<?hh

namespace foo;

class bar {
}
<<__EntryPoint>> function main() {
\class_alias('foo\bar', 'foo\baz');

\var_dump(new namespace\baz);
}
