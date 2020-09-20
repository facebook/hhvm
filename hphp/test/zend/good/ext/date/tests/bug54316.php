<?hh <<__EntryPoint>> function main(): void {
$dt = DateTime::createFromFormat('Y-m-d|', '2011-02-02');
var_dump($dt);

$dt = DateTime::createFromFormat('Y-m-d!', '2011-02-02');
var_dump($dt);
}
