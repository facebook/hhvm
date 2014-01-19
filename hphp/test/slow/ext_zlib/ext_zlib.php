<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

var_dump(readgzfile(__DIR__."/test_ext_zlib.gz"));

VS(gzfile(__DIR__."/test_ext_zlib.gz"), array("Testing Ext Zlib\n"));

VS(gzuncompress(gzcompress("testing gzcompress")), "testing gzcompress");

VS(gzinflate(gzdeflate("testing gzdeflate")), "testing gzdeflate");

$zipped = gzencode("testing gzencode");
$tmpfile = tempnam('/tmp', 'vmzlibtest');
$f = fopen($tmpfile, "w");
fwrite($f, $zipped);
fclose($f);

var_dump(readgzfile($tmpfile));

$zipped = gzencode("testing gzencode");
VS(gzdecode($zipped), "testing gzencode");


$f = gzopen($tmpfile, "w");
VERIFY($f !== false);
gzputs($f, "testing gzputs\n");
gzwrite($f, "<html>testing gzwrite</html>\n");
gzclose($f);

$f = gzopen($tmpfile, "r");
VS(gzread($f, 7), "testing");
VS(gzgetc($f), " ");
VS(gzgets($f), "gzputs\n");
VS(gzgetss($f), "testing gzwrite\n");
VS(gztell($f), 44);
VERIFY(gzeof($f));
VERIFY(gzrewind($f));
VS(gztell($f), 0);
VERIFY(!gzeof($f));
gzseek($f, -7, SEEK_END);
VS(gzgets($f), "testing gzputs\n");
gzclose($f);

$f = gzopen(__DIR__."/test_ext_zlib.gz", "r");
gzpassthru($f);

$compressable = str_repeat('A', 1024);
$s = $compressable;
$t = nzcompress($s);

VERIFY(strlen($t) < strlen($s));

$u = nzuncompress($t);
VS($u, $s);

$compressable = str_repeat('\0', 1024);
$bs = $compressable;
$bt = nzcompress($bs);
VERIFY(strlen($bt) < strlen($bs));
$bu = nzuncompress($bt);
VS($bu, $bs);
VS(count($bu), count($bs));

//////////////////////////////////////////////////////////////////////

$s = "garbage stuff";
$v = nzuncompress($s);
VERIFY($v == false);

$empty = "";
$c = nzcompress($empty);
$d = nzuncompress($c);
VERIFY($d == $empty);

VS(lz4uncompress(lz4compress("testing lz4compress")),
    "testing lz4compress");

VS(lz4uncompress(lz4hccompress("testing lz4hccompress")),
    "testing lz4hccompress");

// first test uncompressing invalid string
$s = "invalid compressed string";
$v = lz4uncompress($s);
VERIFY($v == false);

// try uncompressing empty string
$empty = "";
$v = lz4uncompress($empty);
VERIFY($v == false);

$c = lz4compress($empty);
$d = lz4uncompress($c);
VERIFY($d == $empty);
