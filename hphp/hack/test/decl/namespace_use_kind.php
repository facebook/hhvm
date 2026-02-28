<?hh

use const UseNS\Foo;
use type UseNS\Bar;
use function UseNS\Baz;
use namespace UseNS\NS;

type A = Foo;
type B = Bar;
type C = Baz;
type D = NS\Qux;
