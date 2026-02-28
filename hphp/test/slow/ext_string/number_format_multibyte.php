<?hh


<<__EntryPoint>>
function main_number_format_multibyte() :mixed{
$separators = dict[
  'multichar' => vec['herp', 'derp'],
  'multibyte' => vec['¡', '×']
];

var_dump(
  array_map(
    function($boom) {
      list($dec, $thou) = $boom;
      return number_format(1234.5678, 2, $dec, $thou);
    },
    $separators
  )
);
}
