<?hh
class ancester
{
public $ancester = 0;
	function ancester()
	{
		return $this->ancester;
	}
}
class foo extends ancester
{
public $bar = "1";
	function foo()
	{
		return $this->bar;
	}
}

<<__EntryPoint>>
function main_entry(): void {

  $r = new ReflectionClass('foo');
  foreach ($r->GetProperties() as $p)
  {
  	echo $p->getName(). " ". $p->getDeclaringClass()->getName()."\n";
  }
}
