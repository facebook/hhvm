<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
$count = IntlTimeZone::countEquivalentIDs('Europe/Lisbon');
var_dump($count >= 2);

$count2 = intltz_count_equivalent_ids('Europe/Lisbon');
var_dump($count2 == $count);
echo "==DONE==";
}
