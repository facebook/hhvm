<?php

error_reporting(0);
var_dump(date_create("1942-07-13T25:05:02.00+00:00"));
try {
  new DateTime("1942-07-13T25:05:02.00+00:00");
} catch (Exception $e) {
  var_dump($e->getMessage());
}
