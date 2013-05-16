<?php
  class Foo {
    function foo() {
      
      $s = 'preg_replace() is broken';
      
      var_dump(preg_replace_callback(
              '/broken/',
              array(&$this, 'bar'),
              $s
           ));
    }
    
    function bar() {
      return 'working';
    }
    
  } // of Foo

  $o = new Foo;
?>