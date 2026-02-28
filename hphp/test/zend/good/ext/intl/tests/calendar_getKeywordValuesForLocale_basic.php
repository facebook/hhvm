<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

print_r(
dict(
IntlCalendar::getKeywordValuesForLocale('calendar', 'pt', true)
));
echo "\n";

$var = dict(
intlcal_get_keyword_values_for_locale('calendar', 'pt', false)
);
var_dump(count($var) > 8);
var_dump(in_array('japanese', $var));
echo "==DONE==";
}
