<?php

try {
  new DatePeriod(new DateTime("now"));
} catch (Exception $e) {
  var_dump($e->getMessage());
}
