<?hh

<<__DynamicallyReferenced>> class foo {}
<<__DynamicallyReferenced>> class bar {}

function baz($a, $b, $c, $d, $e) :mixed{
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
    ($e === 'bar' ? HH\classname_to_class('foo') : HH\classname_to_class('bar')) === ($d === 'foo' ? 'foo' : 'bar')
  );
  var_dump(
    ($e === 'bar' ? HH\classname_to_class('foo') : HH\classname_to_class('bar')) !== ($d === 'foo' ? 'foo' : 'bar')
  );
}

<<__EntryPoint>>
function main() :mixed{
  baz(HH\classname_to_class('foo'), HH\classname_to_class('foo'), HH\classname_to_class('bar'), 'foo', 'bar');
}
