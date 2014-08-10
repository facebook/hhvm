<?php
function foo ($a) {
   try {
     throw new Exception("ex");
   } catch (Exception $e) {
     var_dump($a);
     throw $e;
   } finally {
     var_dump("finally");
     return "return";
   }
   return 1;
}

try {
   var_dump(foo("para"));
} catch (Exception $e) {
    "caught exception" . PHP_EOL;
    var_dump($e->getMessage());
}
?>
