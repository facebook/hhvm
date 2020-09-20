<?hh
/* $Id$ */
<<__EntryPoint>> function main(): void {
$xmlstring = '<TEI.2>hello</TEI.2>';
$relaxngfile = dirname(__FILE__) . '/relaxNG.rng';
$file = __SystemLib\hphp_test_tmppath('_007.xml');
file_put_contents($file, $xmlstring);

$reader = new XMLReader();
$reader->open($file);

if ($reader->setRelaxNGSchema($relaxngfile)) {
  while ($reader->read());
}
if ($reader->isValid()) {
  print "file relaxNG: ok\n";
} else {
  print "file relaxNG: failed\n";
}
$reader->close();
unlink($file);


$reader = new XMLReader();
$reader->XML($xmlstring);

if ($reader->setRelaxNGSchema($relaxngfile)) {
  while ($reader->read());
}
if ($reader->isValid()) {
  print "string relaxNG: ok\n";
} else {
  print "string relaxNG: failed\n";
}

$reader->close();

$reader = new XMLReader();
$reader->XML($xmlstring);

if ($reader->setRelaxNGSchema('')) {
    echo 'failed';
}
$reader->close();
echo "===DONE===\n";
}
