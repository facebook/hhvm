<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance('UTC');
$intlcal->setTime(strtotime('2012-02-29 05:06:07 +0000') * 1000);
//print_R($intlcal);
var_dump($intlcal->isSet(IntlCalendar::FIELD_MONTH));
var_dump($intlcal->clear(IntlCalendar::FIELD_MONTH));
var_dump($intlcal->isSet(IntlCalendar::FIELD_MONTH));
//print_R($intlcal);
var_dump(
    $intlcal->getTime(),
    strtotime('2012-01-29 05:06:07 +0000') * 1000.
);
echo "==DONE==";
}
