<?php

$x = 1;
(function ($a) {
  echo __FUNCTION__ . "\n";
})(&$x);
