<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function f() :mixed{
    // During
    var_dump(error_reporting());
    $ret = $y;
    //error_reporting(123);
    return $ret;
}
<<__EntryPoint>>
function entrypoint_error_control(): void {
  error_reporting(-1);

  // Before
  var_dump(error_reporting());

  $x = f();
  //$x = @f();

  /*
  // This is equivalent to the "$x = @f()" statement above (aside from
  // temporary variables)
  $origER = error_reporting();
  error_reporting(0);
  $tmp = f();
  $curER = error_reporting();
  if ($curER === 0) error_reporting($origER);
  $x = $tmp;
  */

  // After
  var_dump(error_reporting());

  echo "Done\n";
}
