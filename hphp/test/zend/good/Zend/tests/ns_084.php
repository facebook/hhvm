<?hh
namespace foo;
use \foo;
class bar {
	function __construct() {echo __METHOD__,"\n";}
}
namespace oops {
class foo {
	function __construct() {echo __METHOD__,"\n";}
}
use foo\bar as foo1;
new foo1;
new foo;
}
<<__EntryPoint>>
function entrypoint_ns_084(): void {
  new foo;
  new bar;
  echo "===DONE===\n";
}
