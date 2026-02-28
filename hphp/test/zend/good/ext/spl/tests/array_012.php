<?hh

class test
{
	public $zero = 0;
	protected $pro;
	public $one = 1;
	private $pri;
	public $two = 2;
}
<<__EntryPoint>>
function main_entry(): void {

  echo "===Array===\n";

  $a = dict['zero' => 0, 'one' => 1, 'two' => 2];
  $it = new ArrayIterator($a);

  foreach($it as $key => $val)
  {
  	echo "$key=>$val\n";
  }

  echo "===Object===\n";

  $o = new test;
  $it = new ArrayIterator($o);

  foreach($it as $key => $val)
  {
  	echo "$key=>$val\n";
  }

  echo "===DONE===\n";
}
