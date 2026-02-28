<?hh

function func1() :mixed{
	return 'func1';
}

function func2() :AsyncGenerator<mixed,mixed,void>{
	yield 'func2';
}


class Foo {
	public function f1() :mixed{
	}

	public function f2() :AsyncGenerator<mixed,mixed,void>{
		yield;
	}
}
<<__EntryPoint>>
function main_entry(): void {

  $closure1 = function() {return "this is a closure"; };
  $closure2 = function($param) {
  	yield $param;
  };

  $rf1 = new ReflectionFunction($closure1);
  var_dump($rf1->isGenerator());

  $rf2 = new ReflectionFunction($closure2);
  var_dump($rf2->isGenerator());

  $rf1 = new ReflectionFunction('func1');
  var_dump($rf1->isGenerator());

  $rf2 = new ReflectionFunction('func2');
  var_dump($rf2->isGenerator());

  $rc = new ReflectionClass('Foo');
  foreach($rc->getMethods() as $m) {
  	var_dump($m->isGenerator());
  }
}
