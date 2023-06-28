<?hh

function perform_test($name, $test_filters, $add_empty) :mixed{
  $params = darray[
    'null' => NULL,
    'empty_array' => varray[],
    'filled_array' => varray[1, 2, 3],
    'int' => 1,
    'double' => 1.0,
    'string' => 'string',
  ];

  if ($add_empty) {
    $name .= ', Add empty';
  }

  echo "$name\n";

  $filters = array_fill_keys(array_keys($params), $test_filters);
  $filters['missing'] = $test_filters;
  var_dump(filter_var_array($params, $filters, $add_empty));
}


<<__EntryPoint>>
function main_filter_var_array() :mixed{
$all_filters = darray[
  'No filters' => varray[],
  'Require Scalar' => darray['flags' => FILTER_REQUIRE_SCALAR],
  'Require Array' => darray['flags' => FILTER_REQUIRE_ARRAY],
];

foreach ($all_filters as $test_name => $filter) {
  perform_test($test_name, $filter, true);
  perform_test($test_name, $filter, false);
}
}
