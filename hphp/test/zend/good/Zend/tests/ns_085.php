<?hh
namespace foo {
	use \foo;
	class bar {
		function __construct() {echo __METHOD__,"\n";}
	}
	function test() {
		new foo;
		new bar;
	}
}

namespace {
	class foo {
		function __construct() {echo __METHOD__,"\n";}
	}
	use foo\bar as foo1;

	function test() {
		new foo1;
		new foo;
		echo "===DONE===\n";
	}

	<<__EntryPoint>>
	function main() {
		\foo\test();
		\test();
	}
}
