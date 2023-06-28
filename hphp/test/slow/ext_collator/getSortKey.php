<?hh

function sort_key_cmp(Collator $c, string $a, string $b) :mixed{
  $ka = $c->getSortKey($a);
  $kb = $c->getSortKey($b);
  if ($ka < $kb) {
    return -1;
  } else if ($ka === $kb) {
    return 0;
  } else {
    return 1;
  }
}


<<__EntryPoint>>
function main_get_sort_key() :mixed{
$inputs = varray[
  varray['1', '2', '10'],
  varray['y', 'k', 'i'],
];

$locales = varray[
  'en_US',
  'lt_LT',
];

foreach ($inputs as $input) {
  foreach ($locales as $locale) {
    $c = new Collator($locale);
    usort(inout $input, function($a, $b) use ($c) {
      return sort_key_cmp($c, $a, $b);
    });
    var_dump(darray[$locale => $input]);
    $c->setAttribute(Collator::NUMERIC_COLLATION, Collator::ON);
    usort(inout $input, function($a, $b) use ($c) {
      return sort_key_cmp($c, $a, $b);
    });
    var_dump(darray[$locale.' numeric' => $input]);
  }
}
}
