<?hh


<<__EntryPoint>>
function main_add_child_empty_ns() :mixed{
foreach (vec[null, "", false, 0] as $type) {
    $xml = simplexml_load_string('<root />');
    $xml->addChild("a", "b", $type);
    var_dump(str_replace(PHP_EOL, "", $xml->a->asXML()));
}
}
