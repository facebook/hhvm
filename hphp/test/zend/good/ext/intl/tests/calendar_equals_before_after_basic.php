<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal1 = new IntlGregorianCalendar(2012, 1, 29, 16, 59, 59);
$intlcal2 = IntlCalendar::createInstance(null, '@calendar=japanese');
$intlcal3 = new IntlGregorianCalendar(2012, 1, 29, 17, 00, 00);
$intlcal2->setTime($intlcal1->getTime());

var_dump($intlcal2->getType());

var_dump("1 eq 1",        $intlcal1->equals($intlcal1));

var_dump("1 eq 2",        $intlcal1->equals($intlcal2));
var_dump("1 before 2",    $intlcal1->before($intlcal2));
var_dump("1 after 2",    $intlcal1->after($intlcal2));

var_dump("1 eq 3",        $intlcal1->equals($intlcal3));
var_dump("1 before 3",    $intlcal1->before($intlcal3));
var_dump("1 after 3",    $intlcal1->after($intlcal3));

var_dump("3 eq 2",        intlcal_equals($intlcal3, $intlcal2));
var_dump("3 before 2",    intlcal_before($intlcal3, $intlcal2));
var_dump("3 after 2",    intlcal_after($intlcal3, $intlcal2));
echo "==DONE==";
}
