<?php

$a = new class {
  function func() {
    $b = new class {
    function func2() {
      $closure = function () {
        echo "done\n";
      };

      $closure();
    }
    };
    $b->func2();
  }
};
$a->func();
