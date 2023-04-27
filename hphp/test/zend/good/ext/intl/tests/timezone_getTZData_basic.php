<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
print_r(IntlTimeZone::getTZDataVersion());
echo "\n";
print_r(intltz_get_tz_data_version());
echo "\n";
echo "==DONE==";
}
