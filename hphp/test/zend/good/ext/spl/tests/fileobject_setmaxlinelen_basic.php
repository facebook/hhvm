<?hh <<__EntryPoint>> function main() {
$s = new SplFileObject( __FILE__ );
$s->setMaxLineLen( 3);
echo $s->getCurrentLine();
}
