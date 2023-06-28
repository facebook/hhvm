<?hh
function foo () :mixed{
   try {
     throw new Exception("try");
   } finally {
     throw new Exception("finally");
   }
}
<<__EntryPoint>> function main(): void {
try {
  foo();
} catch (Exception $e) {
  do {
    var_dump($e->getMessage());
  } while ($e = $e->getPrevious());
}
}
