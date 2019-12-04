<?hh

namespace foo;

class foo {

}
<<__EntryPoint>> function main(): void {


\var_dump(\class_exists('\foo'));
\var_dump(\class_exists('\foo\foo'));
\var_dump(\class_exists('foo\foo'));
\var_dump(\class_exists('foo'));
}
