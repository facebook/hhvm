<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");

$mf = new MessageFormatter('en_US',
    "{0,number} -- {1,ordinal}");

var_dump($mf->format(dict[0 => 1.3, 1 => 1.3]));
var_dump($mf->format(dict[0 => 1.3, 1 => 1.3]));
$mf->setPattern("{0,ordinal} -- {1,number}");
var_dump($mf->format(dict[0 => 1.3, 1 => 1.3]));
echo "==DONE==";
}
