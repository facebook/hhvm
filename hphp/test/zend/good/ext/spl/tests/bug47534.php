<?hh <<__EntryPoint>> function main(): void {
$it1 = new RecursiveDirectoryIterator(dirname(__FILE__), FilesystemIterator::CURRENT_AS_PATHNAME);
$it1->rewind();
echo gettype($it1->current())."\n";

$it2 = new RecursiveDirectoryIterator(dirname(__FILE__));
$it2->rewind();
echo gettype($it2->current())."\n";
}
