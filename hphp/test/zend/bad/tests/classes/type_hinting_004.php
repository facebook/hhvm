<?php
  set_error_handler('myErrorHandler', E_RECOVERABLE_ERROR);
  function myErrorHandler($errno, $errstr, $errfile, $errline) {
      echo "$errno: $errstr - $errfile($errline)\n";
      return true;
  }
  
  echo "---> Type hints with callback function:\n";
  class A  {  }
  function f1(A $a)  {
      echo "in f1;\n";
  }
  function f2(A $a = null)  {
      echo "in f2;\n";
  }
  call_user_func('f1', 1);
  call_user_func('f1', new A);
  call_user_func('f2', 1);
  call_user_func('f2');
  call_user_func('f2', new A);
  call_user_func('f2', null);
  
  
  echo "\n\n---> Type hints with callback static method:\n";
  class C {
      static function f1(A $a) {
          if (isset($this)) {
              echo "in C::f1 (instance);\n";
          } else {
              echo "in C::f1 (static);\n";
          }
      }
      static function f2(A $a = null) {
          if (isset($this)) {
              echo "in C::f2 (instance);\n";
          } else {
              echo "in C::f2 (static);\n";
          }
      }
  }
  call_user_func(array('C', 'f1'), 1);
  call_user_func(array('C', 'f1'), new A);
  call_user_func(array('C', 'f2'), 1);
  call_user_func(array('C', 'f2'));
  call_user_func(array('C', 'f2'), new A);
  call_user_func(array('C', 'f2'), null);
  
  
  echo "\n\n---> Type hints with callback instance method:\n";
  class D {
      function f1(A $a) {
          if (isset($this)) {
              echo "in C::f1 (instance);\n";
          } else {
              echo "in C::f1 (static);\n";
          }
      }
      function f2(A $a = null) {
          if (isset($this)) {
              echo "in C::f2 (instance);\n";
          } else {
              echo "in C::f2 (static);\n";
          }
      }
  }
  $d = new D;
  call_user_func(array($d, 'f1'), 1);
  call_user_func(array($d, 'f1'), new A);
  call_user_func(array($d, 'f2'), 1);
  call_user_func(array($d, 'f2'));
  call_user_func(array($d, 'f2'), new A);
  call_user_func(array($d, 'f2'), null);
  
?>