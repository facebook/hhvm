<?hh

class X {
  function foo() :mixed{
    switch ($this) {
    case 'foo': echo 'foo';
 break;
    case 'bar': echo 'bar';
 break;
    default: echo 'def';
    }
  }
  function bar($arg) :mixed{
    switch ($this) {
    case $arg: echo 'arg';
 break;
    default: echo 'def';
    }
  }
  function baz($arg) :AsyncGenerator<mixed,mixed,void>{
    switch ($this) {
    case $arg: echo 'arg';
 break;
    default: echo 'def';
    }
    yield $arg;
  }
}

<<__EntryPoint>>
function main_1757() :mixed{
$x = new X;
$x->foo();
$x->bar(new stdClass);
$x->bar($x);
foreach ($x->baz($x) as $v) {
  var_dump($v);
}
}
