<?hh
class ancester
{
public $ancester = 0;
	function ancester()
:mixed	{
		return $this->ancester;
	}
}
class foo extends ancester
{
public $bar = "1";
	function foo()
:mixed	{
		return $this->bar;
	}
}

<<__EntryPoint>>
function main_entry(): void {

  $r = new ReflectionClass('foo');
  foreach ($r->getProperties() as $p)
  {
  	echo $p->getName(). " ". $p->getDeclaringClass()->getName()."\n";
  }
}
