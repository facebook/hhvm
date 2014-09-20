<?php

namespace {
	function test() {
		echo "NO!";
	}
}
namespace foo {
	use function bar\test;
	test();
}

?>
