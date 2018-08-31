<?php


<<__EntryPoint>>
function main_lambda_in_nested_anon_class() {
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
}
