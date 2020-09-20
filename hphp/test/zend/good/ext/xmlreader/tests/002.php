<?hh 
<<__EntryPoint>>
function entrypoint_002(): void {
  /* $Id$ */
  $filename = __SystemLib\hphp_test_tmppath('_002.xml');
  $xmlstring = '<?xml version="1.0" encoding="UTF-8"?>
  <books></books>';
  file_put_contents($filename, $xmlstring);

  $reader = new XMLReader();
  if ($reader->open('')) exit();

  $reader = new XMLReader();
  if (!$reader->open($filename)) {
  	$reader->close();
  	exit();
  }

  // Only go through
  while ($reader->read()) {
  	echo $reader->name."\n";
  }
  $reader->close();
  unlink($filename);
  touch($filename);
  $reader = new XMLReader();
  $reader->open($filename);
  $reader->close();
  unlink($filename);

  echo "===DONE===\n";
}
