<?php

function f_str($x) {
  var_dump($x);
  print ' goes to: ';
  switch ($x) {
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
function f_bool($x) {
  var_dump($x);
  print ' goes to: ';
  switch ($x) {
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
function f_dbl($x) {
  var_dump($x);
  print ' goes to: ';
  switch ($x) {
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
function f_dbl_notpe($x) {
  var_dump($x);
  print ' goes to: ';
  switch ($x) {
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
function f_int($x) {
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
function f($x) {
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
  default:
    print 'default';
    break;
  }
}
function fcn($x) {
 if ($x) return 5;
 return 'bar';
 }
function st($x) {
  switch ($y = $x) {
  case 0:
    print '0';
  }
  switch (fcn(true)) {
  case 3:
    print '3';
    break;
  case 7:
  case 5:
    print '5 or 7';
    break;
  }
  switch (++$x) {
  case 1:
    print '1';
  }
  switch ($x + $y + f(true)) {
  case -30:
    print '-30';
    break;
  default:
    print 'default';
  }
  switch($x){
}
  switch($x){
  default:
    print 'default';
  }
}
f(0);
f(-1);
f(1);
f(2);
f(true);
f(false);
f(null);
f(array());
f(1.0);
f('1abc');
f('3');
f('foo');
class M{
}
f(new M());
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
f_dbl(log(0));
f_dbl_notpe('5000000.3920');
f_dbl_notpe('5000000.5');
f_dbl_notpe('5000000.5001');
f_dbl_notpe('5000000.0');
f_int(0x7fffffffffffffff);
f_int(-120);
f_int(0);
st(10);
