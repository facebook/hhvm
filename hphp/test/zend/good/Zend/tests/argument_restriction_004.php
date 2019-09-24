<?hh
class Foo {
}

abstract class Base {
	abstract public function test(Foo $foo, array $bar, $option = NULL, $extra = 16777215) ;
}

class Sub extends Base {
	public function test(Foo $foo, array $bar, $option = NULL, $extra = 0xffffff ) {
	}
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
