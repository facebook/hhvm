<?php
trait T1 {
	function foo() {}
}
class C1 {
	use T1 {
		T1::foo as final;
	}
}
?>