<?hh

class M {
}

<<__EntryPoint>>
function main() :mixed{
  f(0);
  f(-1);
  f(1);
  f(2);
  f(-120);
  f(0x7fffffffffffffff);
  f(true);
  f(false);
  f(null);
  f(vec[]);
  f(1.0);
  f('1abc');
  f('3');
  f('foo');
  f('0');
  f('');
  f('jazz');
  f('5');
  f('1');
  try {
    f(new M());
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }

  f(5.3920);
  f(5.5);
  f(5.5001);
  f(5.0);
  f(log(0.0));
  f('5.3920');
  f('5.5');
  f('5.5001');
  f('5.0');

}

function f($x) :mixed{
  var_dump($x);
  print "goes to:\n";
  switch ($x) {
    case 5:
      print '5';
      break;
    case 1:
      print '1';
      break;
    case 0:
      print '0';
      break;
    default:
      print 'default';
      break;
  }
  echo "\n";
}
