<?hh <<__EntryPoint>> function main(): void {
$r1 = new ReflectionClass("stdClass");
try { var_dump($r1->isInternal('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($r1->isInternal('X', true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
