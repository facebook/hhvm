<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}
function VERIFY($x) { VS($x, true); }

$tmpfile = tempnam('/tmp', 'hhbztest.tmp');

function test_bzwrite() {
  global $tmpfile;

  $str = "HipHop for";
  $bz = bzopen($tmpfile, "w");
  VERIFY($bz !== false);
  VS(bzwrite($bz, $str), 10);
  bzflush($bz);
  VERIFY(bzclose($bz));

  $bz = bzopen($tmpfile, "r");
  $ret = bzread($bz, 10000);
  VS($ret, $str);
  VERIFY(bzclose($bz));
  VS($ret, $str);
  unlink($tmpfile);
}

function test_bzerrstr() {
  global $tmpfile;

  $f = fopen($tmpfile, "w");
  fwrite($f, "this is a test");
  fclose($f);
  $f = bzopen($tmpfile, "r");
  bzread($f);
  $ret = bzerrstr($f);
  bzclose($f);
  unlink($tmpfile);
  VS($ret, "DATA_ERROR_MAGIC");
}

function test_bzerror() {
  global $tmpfile;

  $f = fopen($tmpfile, "w");
  fwrite($f, "this is a test");
  fclose($f);
  $f = bzopen($tmpfile, "r");
  bzread($f);
  $ret = bzerror($f);
  bzclose($f);
  unlink($tmpfile);
  VS($ret, array("errno" => -5,
                 "errstr" => "DATA_ERROR_MAGIC"));
}

test_bzwrite();
test_bzerrstr();
test_bzerror();
