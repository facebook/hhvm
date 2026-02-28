<?hh
function foo () :mixed{
   try {
     echo "try\n";
     return 1;
   } catch (Exception $e) {
   } finally {
     echo "finally\n";
   }
   return 2;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}
