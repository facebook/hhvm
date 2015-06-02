<?hh

$files = Map {
  "file://" . __FILE__ => null,
  "file://" . __FILE__ . ".expectf" => null,
};

$handles = $files->mapWithKey(($file,$dummy) ==> HH\Asio\curl_exec($file));
HH\Asio\join(HH\Asio\m($handles))->mapWithKey(
  function ($filename, $contents) {
    $actual = file_get_contents($filename);
    echo "$filename\n";
    var_dump(is_string($contents));
    var_dump($contents === $actual);
  }
);
