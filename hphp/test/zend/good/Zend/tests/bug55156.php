<?hh

/** test */
namespace foo {
    function test() :mixed{ }

    /** test1 */
    class bar { }

    /** test2 */
    class foo extends namespace\bar { }

    <<__EntryPoint>> function main(): void {
    $x = new \ReflectionFunction('foo\test');
    \var_dump($x->getDocComment());

    $x = new \ReflectionClass('foo\bar');
    \var_dump($x->getDocComment());

    $x = new \ReflectionClass('foo\foo');
    \var_dump($x->getDocComment());
    }
}
