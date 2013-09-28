<?php

class Example {
   function whatever() {
      if (isset($this)) {
          var_dump('static method call');
      }
 else {
          var_dump('non-static method call');
      }
   }
}
Example::whatever();
$inst = new Example();
$inst->whatever();
