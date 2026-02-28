<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::createEnumeration(3600000);
var_dump(get_class($tz));
$count = count(dict($tz));
var_dump($count > 20);

$tz->rewind();
var_dump(in_array('Europe/Amsterdam', dict($tz)));
echo "==DONE==";
}
