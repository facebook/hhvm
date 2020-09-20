<?hh
<<__EntryPoint>> function main(): void {
$x = new splfileinfo(__FILE__);

try {
$x->openFile(NULL, NULL, NULL);
} catch (Exception $e) { }

var_dump($x->getPathName());
}
