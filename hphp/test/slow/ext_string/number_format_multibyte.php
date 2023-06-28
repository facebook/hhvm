<?hh


<<__EntryPoint>>
function main_number_format_multibyte() :mixed{
$separators = darray[
  'multichar' => varray['herp', 'derp'],
  'multibyte' => varray['¡', '×']
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
