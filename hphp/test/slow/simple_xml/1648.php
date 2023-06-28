<?hh
function convert_simplexml_to_array($sxml) :mixed{
  if ($sxml) {
    foreach ($sxml as $k => $v) {
      var_dump($k, (string)$v);
      convert_simplexml_to_array($v);
    }
  }
}


<<__EntryPoint>>
function main_1648() :mixed{
$xml = '<?xml version="1.0" encoding="UTF-8"?><response><t>6</t></response>';
$sxml = simplexml_load_string($xml);
convert_simplexml_to_array($sxml);
}
