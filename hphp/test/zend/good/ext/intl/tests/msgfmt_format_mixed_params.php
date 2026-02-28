<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);
//ini_set("intl.default_locale", "nl");

$mf = new MessageFormatter('en_US',
    "{0,number} -- {foo,ordinal}");

var_dump($mf->format(dict[0 => 2.3, "foo" => 1.3]));
var_dump($mf->format(dict["foo" => 1.3, 0 => 2.3]));
echo "==DONE==";
}
