<?php
$data = <<<YAML
#yaml
---
data: !mytag
  - look upper
...
YAML;

function tag_callback ($value, $tag, $flags) {
  echo "-- callback value --\n";
  var_dump($value);
  var_dump($tag);
  var_dump($flags);
  echo "-- end callback value --\n";
  return array(
      'data' => $value,
      'another' => 'test',
    );
}

/* baseline. do like operation in native php. */
$native = array(
  "data" => array("look upper"),
);
$native["data"] = tag_callback($native["data"], "!mytag", 0);

echo "-- native value --\n";
var_dump($native);
echo "-- end native value --\n";

$cnt = null;
$array = yaml_parse($data, 0, $cnt, array(
    '!mytag' => 'tag_callback',
  ));

echo "-- parsed value --\n";
var_dump($array);
echo "-- end parsed value --\n";
?>
