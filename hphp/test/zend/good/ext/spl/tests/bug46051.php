<?hh
<<__EntryPoint>> function main(): void {
$x = new SplFileInfo(__FILE__);

try {
$x->openFile(NULL, NULL, NULL);
} catch (Exception $e) { }

var_dump($x->getPathname());
}
