<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
$enum = IntlTimeZone::createTimeZoneIDEnumeration(
    IntlTimeZone::TYPE_ANY,
    'PT',
    -3600000);
print_r(dict($enum));

$enum = intltz_create_time_zone_id_enumeration(
    IntlTimeZone::TYPE_ANY,
    'PT',
    -3600000);
print_r(dict($enum));
echo "==DONE==";
}
