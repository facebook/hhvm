<?php
function run_tests($start_key = null, $stop_key = null) {
  $settings = ini_get_all();
  $trimmed_arr = array();
  if ($start_key !== null && $stop_key !== null) {
    $start = array_search($start_key, array_keys($settings));
    $end = array_search($stop_key, array_keys($settings));
    $trimmed_arr = array_slice($settings, $start, $end - $start + 1, true);
  } else if ($start_key !== null) {
    $start = array_search($start_key, array_keys($settings));
    $trimmed_arr = array_slice($settings, $start,
                               count($settings) - $start, true);
  } else if ($stop_key !== null) {
    $end = array_search($stop_key, array_keys($settings));
    $trimmed_arr = array_slice($settings, 0, $end + 1, true);
  }
  var_dump($trimmed_arr);
}
