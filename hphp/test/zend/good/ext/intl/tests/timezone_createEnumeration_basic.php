<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::createEnumeration();
var_dump(get_class($tz));
$count = count(dict($tz));
var_dump($count > 300);

$tz = intltz_create_enumeration();
var_dump(get_class($tz));
$count2 = count(dict($tz));
var_dump($count == $count2);
echo "==DONE==";
}
