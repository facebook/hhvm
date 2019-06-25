<?hh

namespace foo;


class foo {
}
<<__EntryPoint>> function main(): void {
\class_alias(__NAMESPACE__ .'\foo', 'foo');
\class_alias('\foo', 'foo');
}
