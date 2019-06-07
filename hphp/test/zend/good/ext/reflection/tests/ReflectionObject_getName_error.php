<?hh
class C { }
$myInstance = new C;
$r2 = new ReflectionObject($myInstance);

$r3 = new ReflectionObject($r2);

try { var_dump($r3->getName(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($r3->getName('x','y')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump($r3->getName(0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
