<?hh

class C {

}
<<__EntryPoint>>
function entrypoint_ReflectionClass_FileInfo_basic(): void {
  //New instance of class C - defined below
  $rc = new ReflectionClass("C");

  //Get the file name of the PHP script in which C is defined
  var_dump($rc->getFileName());

  //Get the line number at the start of the definition of class C
  var_dump($rc->getStartLine());

  //Get the line number at the end of the definition of class C
  var_dump($rc->getEndLine());

  //Same tests as above but stdclass is internal - so all results should be false.
  $rc = new ReflectionClass("stdClass");
  var_dump($rc->getFileName());
  var_dump($rc->getStartLine());
  var_dump($rc->getEndLine());
}
