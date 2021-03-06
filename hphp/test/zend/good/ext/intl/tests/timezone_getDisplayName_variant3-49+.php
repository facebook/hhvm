<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("error_reporting", -1);
ini_set("display_errors", 1);

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');

ini_set('intl.default_locale', 'en_US');
var_dump($lsb->getDisplayName(false, IntlTimeZone::DISPLAY_LONG));
var_dump($lsb->getDisplayName(false, IntlTimeZone::DISPLAY_LONG, ''));
var_dump($lsb->getDisplayName(false, IntlTimeZone::DISPLAY_LONG, 'pt_PT'));
echo "==DONE==";
}
