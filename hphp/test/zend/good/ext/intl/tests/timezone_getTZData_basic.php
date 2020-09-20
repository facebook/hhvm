<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
print_R(IntlTimeZone::getTZDataVersion());
echo "\n";
print_R(intltz_get_tz_data_version());
echo "\n";
echo "==DONE==";
}
