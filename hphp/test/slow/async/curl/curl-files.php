<?hh

require(__DIR__ . "/async-curl.inc");

$files = [
  "file://" . __FILE__,
  "file://" . __DIR__ . "/async-curl.inc",
];

$ac = new AsyncCurl(...$files);
foreach($ac->gen()->join() as $idx => $contents) {
  $actual = file_get_contents($files[$idx]);
  echo $files[$idx] . "\n";
  var_dump(is_string($contents));
  var_dump($contents === $actual);
}
