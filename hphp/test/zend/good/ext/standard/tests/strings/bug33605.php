<?hh <<__EntryPoint>> function main(): void {
$res = substr_compare("aa", "a", -99999999, -1, false);
var_dump($res);
}
