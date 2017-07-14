<?php
function foo () {
   try {
   } finally {
      goto label;
   }
label:
   return 1;
}

foo();
?>
