<?hh

function cmpCTrue($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == true) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, true));
  print "(true == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(false, $x));
}

function cmpCFalse($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == false) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, false));
  print "(false == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(false, $x));
}

function cmpC0($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 0) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 0));
  print "(0 == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(0, $x));
}

function cmpC1($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 1) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 1));
  print "(1 == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(1, $x));
}

function cmpC2($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 2) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 2));
  print "(2 == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(2, $x));
}

function cmpC3($x) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "(x == 3) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 3));
  print "(3 == x) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq(3, $x));
}

function cmp3($x, $y) :mixed{
  print "----------\n";
  print "x = ";
  var_dump($x);
  print "y = ";
  var_dump($y);
  print "(x == y) = ";
  var_dump(HH\Lib\Legacy_FIXME\eq($x, $y));
  print "(x != y) = ";
  var_dump(HH\Lib\Legacy_FIXME\neq($x, $y));
}

function cmp2($x, $y) :mixed{
  cmp3($x, $y);
  cmp3($y, $x);
}

function cmp1($x) :mixed{
  cmp2($x, true);
  cmp2($x, false);
  cmp2($x, 0);
  cmp2($x, 1);
  cmp2($x, 2);
  cmp2($x, 3);
  cmpCTrue($x);
  cmpCFalse($x);
  cmpC0($x);
  cmpC1($x);
  cmpC2($x);
  cmpC3($x);
}

<<__EntryPoint>> function cmp(): void {
  cmp1(true);
  cmp1(false);
  cmp1(0);
  cmp1(1);
  cmp1(2);
  cmp1(1234567);
}
