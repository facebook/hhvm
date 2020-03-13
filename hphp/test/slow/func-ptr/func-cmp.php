<?hh

function foo() {}
function bar() {}

function baz($a, $b, $c, $d, $e) {
  var_dump($a === $b); var_dump($a == $b);
  var_dump($a < $b);   var_dump($a <= $b);
  var_dump($a > $b);   var_dump($a >= $b);

  var_dump($a === $c); var_dump($a == $c);
  var_dump($a < $c);   var_dump($a <= $c);
  var_dump($a > $c);   var_dump($a >= $c);

  var_dump($a === $d); var_dump($a == $d);
  var_dump($a < $d);   var_dump($a <= $d);
  var_dump($a > $d);   var_dump($a >= $d);

  var_dump($a === $e); var_dump($a == $e);
  var_dump($a < $e);   var_dump($a <= $e);
  var_dump($a > $e);   var_dump($a >= $e);

  var_dump(
    ($e === 'bar' ? fun('foo') : fun('bar')) === ($d === 'foo' ? 'foo' : 'bar')
  );
  var_dump(
    ($e === 'bar' ? fun('foo') : fun('bar')) !== ($d === 'foo' ? 'foo' : 'bar')
  );
}

<<__EntryPoint>>
function main() {
  baz(fun('foo'), fun('foo'), fun('bar'), 'foo', 'bar');
}
