<?php
namespace test;

function a(\Throwable $t) {
  var_dump(get_class($t));
}

a(new \RuntimeException('foo'));
