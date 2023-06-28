<?hh

class ArrayIteratorEx extends ArrayIterator
{
	function rewind()
:mixed	{
		echo __METHOD__ . "\n";
		return parent::rewind();
	}
}

<<__EntryPoint>>
function main_entry(): void {

  $it = new ArrayIteratorEx(range(0,3));

  foreach(new IteratorIterator($it) as $v)
  {
  	var_dump($v);
  }

  echo "===DONE===\n";
}
