<?hh
class test2 {
	private static $instances = 0;
	public $instance;

	public function __construct() {
		$this->instance = ++self::$instances;
	}

}
<<__EntryPoint>>
function main_entry(): void {

  $foo = new test2();

  if (is_object($foo)) {
    include 'bug39721.inc';
  }

  $child = new test2_child();

  echo $foo->instance . "\n";
  echo $child->instance . "\n";
}
