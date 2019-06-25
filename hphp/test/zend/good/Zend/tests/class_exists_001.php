<?hh

namespace foo;

class foo {

}
<<__EntryPoint>> function main(): void {
\class_alias(__NAMESPACE__ .'\foo', 'bar');


\var_dump(\class_exists('\bar'));
\var_dump(\class_exists('bar'));
\var_dump(\class_exists('foo\bar'));
\var_dump(\class_exists('foo\foo'));
\var_dump(\class_exists('foo'));
}
