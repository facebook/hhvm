<?hh
class C { }
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass("C");
$methods = vec["getFileName", "getStartLine", "getEndLine"];

foreach ($methods as $method) {
    var_dump($rc->$method());
    try { var_dump($rc->$method(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
    try { var_dump($rc->$method('X', 0));     } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}
