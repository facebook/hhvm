<?hh

function eq($x, $y) :mixed{
  var_dump(HH\Lib\Legacy_FIXME\eq($x, $y));
}

function lt($x, $y) :mixed{
  try {
    var_dump(HH\Lib\Legacy_FIXME\lt($x, $y));
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

function gt($x, $y) :mixed{
  try {
    var_dump(HH\Lib\Legacy_FIXME\gt($x, $y));
  } catch (Exception $e) {
    echo "Caught: ".$e->getMessage()."\n";
  }
}

<<__EntryPoint>> function main(): void {
echo "======\n";

eq('Array', vec[1,2]);
eq('Array', vec[]);
eq(vec[], 'Array');
eq(vec['a', 'b'], 'Array');
echo "\n";
lt('Array', vec[1,2]);
lt('Array', vec[]);
lt(vec[], 'Array');
lt(vec['a', 'b'], 'Array');
echo "\n";
gt('Array', vec[1,2]);
gt('Array', vec[]);
gt(vec[], 'Array');
gt(vec['a', 'b'], 'Array');

echo "======\n";

eq('', null);
eq(null, null);
eq(null, '');
eq('', '');
echo "\n";
lt('', null);
lt(null, null);
lt(null, '');
lt('', '');
echo "\n";
gt('', null);
gt(null, null);
gt(null, '');
gt('', '');

echo "======\n";

eq(-1.0, null);
eq(null, -1.0);
echo "\n";
lt(-1.0, null);
lt(null, -1.0);
echo "\n";
gt(-1.0, null);
gt(null, -1.0);
}
