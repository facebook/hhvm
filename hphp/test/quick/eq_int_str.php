<?hh

function ci($x) :mixed{
  print "----------\nx == 0\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 0));

  print "----------\nx == 123\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 123));

  print "----------\nx == -456\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, -456));

  print "----------\nx == 7.8\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 7.8));

  print "----------\nx == 90000000000\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, 90000000000));
}

function cs($x) :mixed{
  print "----------\nx == ''\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, ""));

  print "----------\nx == '123'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "123"));

  print "----------\nx == '123q'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "123q"));

  print "----------\nx == 'q123'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "q123"));

  print "----------\nx == '123.0'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "123.0"));

  print "----------\nx == '-456'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "-456"));

  print "----------\nx == '-456.7'\n";
  var_dump($x);
  var_dump($x == "-456.7");

  print "----------\nx == '7.80'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "7.80"));

  print "----------\nx == '90000000000'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "90000000000"));

  print "----------\nx == '9e10'\n";
  var_dump($x);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, "9e10"));
}

function is($x, $y) :mixed{
  print("----------\nx == y\n");
  var_dump($x);
  var_dump($y);
  var_dump(HH\Lib\Legacy_FIXME\eq($x, $y));
}

<<__EntryPoint>> function cmpr(): void {
  $s = vec[
    "",
    "123",
    "123q",
    "q123",
    "-456",
    "-456.7",
    "7.80",
    "9000000000",
    "9e10",
  ];
  $i = vec[
    0, 123, -456, 7.8, 90000000000,
  ];

  for ($ji = 0; $ji < 5; ++$ji) {
    cs($i[$ji]);
  }
  for ($js = 0; $js < 9; ++$js) {
    ci($s[$js]);
  }

  for ($ji = 0; $ji < 5; ++$ji) {
    for ($js = 0; $js < 9; ++$js) {
      is($i[$ji], $s[$js]);
    }
  }

  print "----------\n0 == 'q123'\n"; 
  var_dump(HH\Lib\Legacy_FIXME\eq(0, "q123"));
  print "----------\n123 == '123q'\n"; 
  var_dump(HH\Lib\Legacy_FIXME\eq(123, "123q"));
  print "----------\n123 == '123.0'\n";
  var_dump(HH\Lib\Legacy_FIXME\eq(123, "123.0"));
  print "----------\n90000000000 == '9e10'\n";
  var_dump(9000000000 == "9e10");
  print "----------\n0 == '-456'\n"; 
  var_dump(0 == "-456");
}
