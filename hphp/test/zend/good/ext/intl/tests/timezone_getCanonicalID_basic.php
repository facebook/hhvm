<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
$is_system_id = false;
print_R(IntlTimeZone::getCanonicalID('Portugal', inout $is_system_id));
echo "\n";
print_R(intltz_get_canonical_id('Portugal', inout $is_system_id));
echo "\n";
echo "==DONE==";
}
