<?hh <<__EntryPoint>> function main(): void {
ini_set("intl.error_level", E_WARNING);

$fmt = <<<EOD
{0} {1}
EOD;

$mf = new MessageFormatter('en_US', $fmt);
var_dump($mf->format(dict[0 => 7]));
}
