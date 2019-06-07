<?hh <<__EntryPoint>> function main() {
ini_set("intl.error_level", E_WARNING);

$fmt = <<<EOD
{0} {1}
EOD;

$mf = new MessageFormatter('en_US', $fmt);
var_dump($mf->format(array(7)));
}
