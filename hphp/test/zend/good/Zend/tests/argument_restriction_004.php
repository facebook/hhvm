<?hh
class Foo {
}

abstract class Base {
	abstract public function test(Foo $foo, AnyArray $bar, $option = NULL, $extra = 16777215) :mixed;
}

class Sub extends Base {
	public function test(Foo $foo, AnyArray $bar, $option = NULL, $extra = 0xffffff ) :mixed{
	}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
