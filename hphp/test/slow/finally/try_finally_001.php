<?hh
function foo ($a) {
   try {
     throw new Exception("ex");
   } finally {
     var_dump($a);
   }
}
<<__EntryPoint>> function main(): void {
foo("finally");
}
