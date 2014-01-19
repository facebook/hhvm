<?php

function a() {
  $b = function ($c) {
    static $d = 0;
    var_dump($c + $d++);
  };
  $b(10);
  $b(20);
  $b(30);
}
a();
