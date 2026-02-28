<?hh

function f_str($x) :mixed{
  var_dump($x);
  print ' goes to: ';
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch($x, -1)) {
  case -1:
    print '-1';
    break;
  case 3:
    print '3';
    break;
  case 0:
    print '0';
    break;
  default:
    print 'default';
    break;
  }
}
function f_bool($x) :mixed{
  var_dump($x);
  print ' goes to: ';
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch($x, -10)) {
  case -10:
    print '-10';
    break;
  case 3:
    print '3';
    break;
  case 0:
    print '0';
    break;
  default:
    print 'default';
    break;
  }
}
function f_dbl($x) :mixed{
  var_dump($x);
  print ' goes to: ';
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch($x, 5000000)) {
  case 5000000:
    print '5000000';
    break;
  case 30:
    print '30';
    break;
  case 0:
    print '0';
    break;
  default:
    print 'default';
    break;
  }
}
function f_dbl_notpe($x) :mixed{
  var_dump($x);
  print ' goes to: ';
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch($x, 5000000)) {
  case 5000000:
    print '5000000';
    break;
  case 30:
    print '30';
    break;
  case 0:
    print '0';
    break;
  default:
    print 'default';
    break;
  }
}
function f_int($x) :mixed{
  var_dump($x);
  print ' goes to: ';
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
  case 300:
    print '300';
    break;
  default:
    print 'default';
    break;
  }
}
function f($x) :mixed{
  var_dump($x);
  print ' goes to: ';
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch($x, 5)) {
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
}
function fcn($x) :mixed{
 if ($x) return 5;
 return 'bar';
 }
function st($x) :mixed{
  switch ($y = $x) {
  case 0:
    print '0';
  default: break;
  }
  switch (fcn(true)) {
  case 3:
    print '3';
    break;
  case 7:
  case 5:
    print '5 or 7';
    break;
  default: break;
  }
  switch (++$x) {
  case 1:
    print '1';
  default: break;
  }
  switch ($x + HH\Lib\Legacy_FIXME\cast_for_arithmetic($y) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(f(true))) {
  case -30:
    print '-30';
    break;
  default:
    print 'default';
  }
  switch($x){
  default:
    print 'default';
  }
}
class M{
}

<<__EntryPoint>>
function main_1754() :mixed{
f(0);
f(-1);
f(1);
f(2);
f(true);
f(false);
f(null);
f(vec[]);
f(1.0);
f('1abc');
f('3');
f('foo');
try {
  f(new M());
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
f_str('0');
f_str('');
f_str('jazz');
f_str('-1');
f_str('1');
f_bool(true);
f_bool(false);
f_dbl(5000000.3920);
f_dbl(5000000.5);
f_dbl(5000000.5001);
f_dbl(5000000.0);
f_dbl(log(0.0));
f_dbl_notpe('5000000.3920');
f_dbl_notpe('5000000.5');
f_dbl_notpe('5000000.5001');
f_dbl_notpe('5000000.0');
f_int(0x7fffffffffffffff);
f_int(-120);
f_int(0);
st(10);
}
