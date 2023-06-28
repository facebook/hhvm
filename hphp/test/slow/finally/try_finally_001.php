<?hh
function foo ($a) :mixed{
   try {
     throw new Exception("ex");
   } finally {
     var_dump($a);
   }
}
<<__EntryPoint>> function main(): void {
foo("finally");
}
