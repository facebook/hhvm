<?hh
class C { function f() :mixed{} }
<<__EntryPoint>>
function entrypoint_ReflectionMethod_getDocComment_error(): void {
  $rc = new ReflectionMethod('C::f');
  try { var_dump($rc->getDocComment(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump($rc->getDocComment('X')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
