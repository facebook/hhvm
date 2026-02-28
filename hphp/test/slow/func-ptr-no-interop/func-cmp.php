<?hh

function foo() :mixed{}
function bar() :mixed{}

function W($f) :mixed{
  try {
    var_dump($f());
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function baz($a, $b, $c, $d, $e) :mixed{
  W(() ==> $a === $b); W(() ==> $a == $b);
  W(() ==> $a < $b);   W(() ==> $a <= $b);
  W(() ==> $a > $b);   W(() ==> $a >= $b);

  W(() ==> $a === $c); W(() ==> $a == $c);
  W(() ==> $a < $c);   W(() ==> $a <= $c);
  W(() ==> $a > $c);   W(() ==> $a >= $c);

  W(() ==> $a === $d); W(() ==> $a == $d);
  W(() ==> $a < $d);   W(() ==> $a <= $d);
  W(() ==> $a > $d);   W(() ==> $a >= $d);

  W(() ==> $a === $e); W(() ==> $a == $e);
  W(() ==> $a < $e);   W(() ==> $a <= $e);
  W(() ==> $a > $e);   W(() ==> $a >= $e);

  W(() ==> 
    ($e === 'bar' ? foo<> : bar<>) === ($d === 'foo' ? 'foo' : 'bar')
  );
  W(() ==> 
    ($e === 'bar' ? foo<> : bar<>) !== ($d === 'foo' ? 'foo' : 'bar')
  );
}

<<__EntryPoint>>
function main() :mixed{
  baz(foo<>, foo<>, bar<>, 'foo', 'bar');
}
