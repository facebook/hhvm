<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');

ini_set('intl.default_locale', 'en_US');
var_dump($lsb->getDisplayName());

ini_set('intl.default_locale', 'pt_PT');
var_dump($lsb->getDisplayName());
echo "==DONE==";
}
