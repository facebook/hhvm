<?hh <<__EntryPoint>> function main(): void {
$rc = new ReflectionFunction('hphp_set_iostatus_address');
var_dump($rc->isDeprecated());
}
