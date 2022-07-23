<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
print_r(IntlTimeZone::getRegion('Europe/Amsterdam'));
echo "\n";
print_r(intltz_get_region('Europe/Amsterdam'));
echo "\n";
echo "==DONE==";
}
