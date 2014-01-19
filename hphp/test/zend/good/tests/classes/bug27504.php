<?php
	class foo {
		function __construct () {
			$this->bar('1');
		}
		private function bar ( $param ) {
			echo 'Called function foo:bar('.$param.')'."\n";
		}
	}

	$foo = new foo();

	call_user_func_array( array( $foo , 'bar' ) , array( '2' ) );

	$foo->bar('3');
?>