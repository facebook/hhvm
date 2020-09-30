<?hh
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

ZendGoodExtIntlTestsDateformatGetSetTimezone::$ts = strtotime('2012-01-01 00:00:00 UTC');

function d(IntlDateFormatter $df) {

echo $df->format(ZendGoodExtIntlTestsDateformatGetSetTimezone::$ts), "\n";
var_dump(
$df->getTimezoneID(),
$df->getTimezone()->getID());
echo "\n";
}

$df = new IntlDateFormatter('pt_PT', 0, 0, 'Europe/Minsk');
d($df);

$df->setTimezone(NULL);
d($df);

$df->setTimezone('Europe/Madrid');
d($df);

$df->setTimezone(IntlTimeZone::createTimeZone('Europe/Paris'));
d($df);

$df->setTimezone(new DateTimeZone('Europe/Amsterdam'));
d($df);

abstract final class ZendGoodExtIntlTestsDateformatGetSetTimezone {
  public static $ts;
}

echo "==DONE==";
