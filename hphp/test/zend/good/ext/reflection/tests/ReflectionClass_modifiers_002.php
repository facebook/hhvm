<?hh
class C {}

<<__EntryPoint>>
function entrypoint_ReflectionClass_modifiers_002(): void {
  $rc = new ReflectionClass("C");
  try { var_dump($rc->isFinal('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->isInterface(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->isAbstract(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->getModifiers(vec[1,2,3])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
