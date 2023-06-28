<?hh

function foo($x, inout $y, $z, inout $q, $r, inout $s) :mixed{
  var_dump($x, $y, $z, $q, $r, $s);
  $y = 'Hello';
  $q = ', ';
  $s = 'World';
  return '!';
}

function bar(inout $a) :mixed{
  try {
    echo "bar($a)\n";
    throw new Exception();
  } catch (Exception $e) {
    try {
      return $a++;
    } finally {
      echo "inner finally\n";
      var_dump($e->getTrace()[0]['function']);
    }
  } finally {
    echo "outer finally\n";
  }
}

function baz($x, inout $a) :mixed{
  echo "baz($x, $a)\n";
  $a = $x;
  if ($x === 42) throw new Exception();
  return $x + 1;
}

function swap(inout $a, inout $b) :mixed{
  $t = $b;
  $b = $a;
  $a = $t;
  // implicit return
}

function empty_(inout $t) :mixed{
  try {
    new stdClass;
    return;
  } finally {
    echo "empty_ finally\n";
  }
}

function main() :mixed{
  $one = 'Eat';
  $two = ' my ';
  $three = 'shorts';
  $four = foo('apple', inout $one, 'orange', inout $two, $two, inout $three);
  echo "$one$two$three$four\n";

  $v = 42;
  var_dump(bar(inout $v), bar(inout $v), $v, bar(inout $v), $v);

  $q = 41;
  try {
    baz(baz($q, inout $q), inout $q);
  } catch (Exception $e) {
    var_dump(array_map($a ==> $a['function'], array_slice($e->getTrace(),0,2)));
    var_dump($q);
  }

  $a = 'alpha';
  $b = 'omega';
  empty_(inout $a);
  empty_(inout $b);
  swap(inout $a, inout $b);
  echo "$a $b\n";
}


<<__EntryPoint>>
function main_call_static() :mixed{
main();
}
