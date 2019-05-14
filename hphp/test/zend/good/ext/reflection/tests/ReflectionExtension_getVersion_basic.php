<?php <<__EntryPoint>> function main() {
$obj = new ReflectionExtension('reflection');
$var = $obj->getVersion() ? $obj->getVersion() : null;
$test = floatval($var) == $var ? true : false;
var_dump($test);
echo "==DONE==";
}
