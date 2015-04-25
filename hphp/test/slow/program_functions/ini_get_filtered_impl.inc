<?php
function run_tests(callable $filter_fn) {
  $settings = ini_get_all();
  $trimmed_arr = array_filter($settings, $filter_fn);
  var_dump($trimmed_arr);
}
