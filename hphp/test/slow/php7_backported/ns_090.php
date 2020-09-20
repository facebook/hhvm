<?hh
namespace Foo\Bar\Baz {
    function foo(){echo __FUNCTION__,"\n";}
    function bar(){echo __FUNCTION__,"\n";}
    const FOO = 0;
    const BAR = 1;
    class A { function __construct() {echo __METHOD__,"\n";} }
    class B { function __construct() {echo __METHOD__,"\n";} }
    class C { function __construct() {echo __METHOD__,"\n";} }
    class D { function __construct() {echo __METHOD__,"\n";} }
}
namespace Fiz\Biz\Buz {

    use Foo\Bar\Baz\{ A, B, C as AC, D };
    use Foo\Bar\Baz\{ function foo, function bar as buz, const FOO, const BAR as BOZ };

    class C { function __construct() {echo __METHOD__,"\n";}}
    function bar(){echo __FUNCTION__,"\n";}
    const BAR = 100;

    <<__EntryPoint>> function main(): void {
    new A;
    new B;
    new AC;
    new D;
    new C;
    foo();
    buz();
    bar();
    \var_dump(FOO);
    \var_dump(BOZ);
    \var_dump(BAR);
    }
}
