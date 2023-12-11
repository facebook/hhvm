<?hh

function perform_test($name, $test_filters, $add_empty) :mixed{
  $params = dict[
    'null' => NULL,
    'empty_array' => vec[],
    'filled_array' => vec[1, 2, 3],
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
$all_filters = dict[
  'No filters' => vec[],
  'Require Scalar' => dict['flags' => FILTER_REQUIRE_SCALAR],
  'Require Array' => dict['flags' => FILTER_REQUIRE_ARRAY],
];

foreach ($all_filters as $test_name => $filter) {
  perform_test($test_name, $filter, true);
  perform_test($test_name, $filter, false);
}
}
