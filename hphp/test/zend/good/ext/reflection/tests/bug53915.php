<?hh
class Foo
{
	const A = 1;
	const B = self::A;
}

class Foo2
{
        const A = 1;
        const B = self::A;
}
<<__EntryPoint>>
function main_entry(): void {

  $rc = new ReflectionClass('Foo');
  print_r($rc->getConstants());

  $rc = new ReflectionClass('Foo2');
  print_r($rc->getConstant('B'));
}
