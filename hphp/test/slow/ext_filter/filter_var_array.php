<?php

$all_filters = array(
  'No filters' => array(),
  'Require Scalar' => array('flags' => FILTER_REQUIRE_SCALAR),
  'Require Array' => array('flags' => FILTER_REQUIRE_ARRAY),
);

function perform_test($name, $test_filters, $add_empty) {
  $params = array(
    'null' => NULL,
    'empty_array' => array(),
    'filled_array' => array(1, 2, 3),
    'int' => 1,
    'double' => 1.0,
    'string' => 'string',
  );

  if ($add_empty) {
    $name .= ', Add empty';
  }

  echo "$name\n";

  $filters = array_fill_keys(array_keys($params), $test_filters);
  $filters['missing'] = $test_filters;
  var_dump(filter_var_array($params, $filters, $add_empty));
}

foreach ($all_filters as $test_name => $filter) {
  perform_test($test_name, $filter, true);
  perform_test($test_name, $filter, false);
}
