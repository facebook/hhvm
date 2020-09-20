<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");
ini_set('date.timezone', 'Europe/Lisbon');

$cal = new IntlGregorianCalendar(2012,04,17,17,35,36);

$dt = $cal->toDateTime();

var_dump($dt->format("c"), $dt->getTimeZone()->getName());
echo "==DONE==";
}
