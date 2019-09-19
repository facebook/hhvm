<?hh

class ArrayIteratorEx extends ArrayIterator
{
	function rewind()
	{
		echo __METHOD__ . "\n";
		return parent::rewind();
	}
}

class ArrayObjectEx extends ArrayObject
{
	function getIterator()
	{
		echo __METHOD__ . "\n";
		return parent::getIterator();
	}
}
<<__EntryPoint>>
function main_entry(): void {

  $it = new ArrayIteratorEx(range(0,3));

  foreach(new IteratorIterator($it) as $v)
  {
  	var_dump($v);
  }

  $it = new ArrayObjectEx(range(0,3));

  foreach(new IteratorIterator($it) as $v)
  {
  	var_dump($v);
  }

  echo "===DONE===\n";
}
