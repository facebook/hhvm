<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance('UTC');
$intlcal->setTime(strtotime('2012-02-29 05:06:07 +0000') * 1000);
var_dump(
        $intlcal->getLeastMaximum(IntlCalendar::FIELD_DAY_OF_MONTH),
        intlcal_get_least_maximum($intlcal, IntlCalendar::FIELD_DAY_OF_MONTH),
        $intlcal->getActualMaximum(IntlCalendar::FIELD_DAY_OF_MONTH),
        intlcal_get_actual_maximum($intlcal, IntlCalendar::FIELD_DAY_OF_MONTH),
        $intlcal->getMaximum(IntlCalendar::FIELD_DAY_OF_MONTH),
        intlcal_get_maximum($intlcal, IntlCalendar::FIELD_DAY_OF_MONTH)
);
echo "==DONE==";
}
