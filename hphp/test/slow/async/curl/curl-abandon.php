<?hh

require(__DIR__ . "/async-curl.inc");

$files = [
  "file://" . __FILE__,
  "file://" . __DIR__ . "/async-curl.inc",
];

$ac = new AsyncCurl(...$files);
$awaitable = $ac->gen();

// Make sure things cleanup when curl_multi_await is abandoned
echo "Done.\n";
