<?hh
function foo ($throw = FALSE) :mixed{
   try {
     echo "try\n";
     if ($throw) {
        throw new Exception("ex");
     }
   } catch (Exception $e) {
     echo "catch\n";
   } finally {
     echo "finally\n";
   }

   echo "end\n";
}
<<__EntryPoint>> function main(): void {
foo();
echo "\n";
foo(true);
}
