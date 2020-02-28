<?hh

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}
function VERIFY($x) { VS($x, true); }

ExtBzip2ExtBzip2Php::$tmpfile = tempnam('/tmp', 'hhbztest.tmp');

function test_bzwrite() {


  $str = "HipHop for";
  $bz = bzopen(ExtBzip2ExtBzip2Php::$tmpfile, "w");
  VERIFY($bz !== false);
  VS(bzwrite($bz, $str), 10);
  bzflush($bz);
  VERIFY(bzclose($bz));

  $bz = bzopen(ExtBzip2ExtBzip2Php::$tmpfile, "r");
  $ret = bzread($bz, 10000);
  VS($ret, $str);
  VERIFY(bzclose($bz));
  VS($ret, $str);
  unlink(ExtBzip2ExtBzip2Php::$tmpfile);
}

function test_bzerrstr() {


  $f = fopen(ExtBzip2ExtBzip2Php::$tmpfile, "w");
  fwrite($f, "this is a test");
  fclose($f);
  $f = bzopen(ExtBzip2ExtBzip2Php::$tmpfile, "r");
  bzread($f);
  $ret = bzerrstr($f);
  bzclose($f);
  unlink(ExtBzip2ExtBzip2Php::$tmpfile);
  VS($ret, "DATA_ERROR_MAGIC");
}

function test_bzerror() {


  $f = fopen(ExtBzip2ExtBzip2Php::$tmpfile, "w");
  fwrite($f, "this is a test");
  fclose($f);
  $f = bzopen(ExtBzip2ExtBzip2Php::$tmpfile, "r");
  bzread($f);
  $ret = bzerror($f);
  bzclose($f);
  unlink(ExtBzip2ExtBzip2Php::$tmpfile);
  VS($ret, darray["errno" => -5,
                 "errstr" => "DATA_ERROR_MAGIC"]);
}

test_bzwrite();
test_bzerrstr();
test_bzerror();

abstract final class ExtBzip2ExtBzip2Php {
  public static $tmpfile;
}
