<?php
function foo () {
   try {
     throw new Exception("try");
   } finally {
     throw new Exception("finally");
   }
}

try {
  foo();
} catch (Exception $e) {
  do {
    var_dump($e->getMessage());
  } while ($e = $e->getPrevious());
}
?>