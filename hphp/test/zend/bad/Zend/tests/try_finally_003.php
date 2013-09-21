<?php
function foo () {
   try {
      echo "1";
      try {
        echo "2";
        throw new Exception("ex");
      } finally {
        echo "3";
      }
   } finally {
      echo "4";
   }
}

foo();
?>