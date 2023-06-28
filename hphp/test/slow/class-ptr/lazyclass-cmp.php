<?hh

class foo {}
class bar {}

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
    ($e === 'bar' ? foo::class : bar::class) === ($d === 'foo' ? 'foo' : 'bar')
  );
  var_dump(
    ($e === 'bar' ? foo::class : bar::class) !== ($d === 'foo' ? 'foo' : 'bar')
  );
}

<<__EntryPoint>>
function main() :mixed{
  baz(foo::class, foo::class, bar::class, 'foo', 'bar');
}
