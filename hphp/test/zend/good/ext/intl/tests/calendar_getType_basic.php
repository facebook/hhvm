<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance();
var_dump($intlcal->getType());
$intlcal = IntlCalendar::createInstance(null, "nl_NL@calendar=hebrew");
var_dump(intlcal_get_type($intlcal));
echo "==DONE==";
}
