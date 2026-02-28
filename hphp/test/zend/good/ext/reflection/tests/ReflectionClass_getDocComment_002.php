<?hh
class C {}
<<__EntryPoint>>
function entrypoint_ReflectionClass_getDocComment_002(): void {
  $rc = new ReflectionClass('C');
  try { var_dump($rc->getDocComment(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->getDocComment('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->getDocComment(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->getDocComment(vec[1,2,3])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
