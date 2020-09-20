<?hh <<__EntryPoint>> function main(): void {
$s = new SplFileObject( __FILE__ );
var_dump($s->getChildren());
}
