<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
print_r(IntlTimeZone::getEquivalentID('Europe/Lisbon', 1));
echo "\n";
print_r(intltz_get_equivalent_id('Europe/Lisbon', 1));
echo "\n";
echo "==DONE==";
}
