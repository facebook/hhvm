<?hh <<__EntryPoint>> function main(): void {
$s = new SplFileObject( __FILE__ );
$s->setMaxLineLen( 3);
echo $s->getCurrentLine();
}
