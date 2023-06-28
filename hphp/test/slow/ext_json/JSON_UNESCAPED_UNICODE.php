<?hh


<<__EntryPoint>>
function main_jso_n_unescape_d_unicode() :mixed{
$test = html_entity_decode('&#x1D11E;');

var_dump($test);
var_dump(json_encode($test));
var_dump(json_encode($test, JSON_UNESCAPED_UNICODE));
}
