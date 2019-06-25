<?hh <<__EntryPoint>> function main(): void {
$rc = new ReflectionFunction('ereg');
var_dump($rc->isDeprecated());
}
