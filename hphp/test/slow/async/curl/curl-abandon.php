<?hh

$files = Map {
  "file://" . __FILE__ => null,
  "file://" . __FILE__ . ".expectf" => null,
};

$handles = $files->mapWithKey(($file, $dummy) ==> HH\Asio\curl_exec($file));

// Make sure things cleanup when curl_multi_await is abandoned
echo "Done.\n";
