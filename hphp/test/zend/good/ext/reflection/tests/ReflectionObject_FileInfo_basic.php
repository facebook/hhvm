<?hh

class C {

}
<<__EntryPoint>>
function entrypoint_ReflectionObject_FileInfo_basic(): void {
  $rc = new ReflectionObject(new C);
  var_dump($rc->getFileName());
  var_dump($rc->getStartLine());
  var_dump($rc->getEndLine());

  $rc = new ReflectionObject(new stdClass);
  var_dump($rc->getFileName());
  var_dump($rc->getStartLine());
  var_dump($rc->getEndLine());
}
