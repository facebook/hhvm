<?php
function foo ($a) {
   try {
     throw new Exception("ex");
   } finally {
     var_dump($a);
   }
}

foo("finally");
?>
