<?php
class Foo {
}

Abstract Class Base {
	abstract public function test(Foo $foo, array $bar, $option = NULL, $extra = 16777215) ;
}

class Sub extends Base {
	public function test(Foo $foo, array $bar, $option = NULL, $extra = 0xffffff ) {
	}	
}
?>
