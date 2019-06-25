<?hh <<__EntryPoint>> function main(): void {
$file = __FILE__;
$s = new SplFileObject( __FILE__ );
echo $s->getBasename();
}
