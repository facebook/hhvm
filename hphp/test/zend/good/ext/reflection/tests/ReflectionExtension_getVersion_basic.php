<?hh <<__EntryPoint>> function main(): void {
$obj = new ReflectionExtension('reflection');
$var = $obj->getVersion() ? $obj->getVersion() : null;
$test = HH\Lib\Legacy_FIXME\eq(floatval($var), $var) ? true : false;
var_dump($test);
echo "==DONE==";
}
