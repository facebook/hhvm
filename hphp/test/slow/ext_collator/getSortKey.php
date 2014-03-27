<?hh

$inputs = array(
  array('1', '2', '10'),
  array('y', 'k', 'i'),
);

$locales = array(
  'en_US',
  'lt_LT',
);

function sort_key_cmp(Collator $c, string $a, string $b) {
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

foreach ($inputs as $input) {
  foreach ($locales as $locale) {
    $c = new Collator($locale);
    usort($input, function($a, $b) use ($c) {
      return sort_key_cmp($c, $a, $b);
    });
    var_dump(array($locale => $input));
    $c->setAttribute(Collator::NUMERIC_COLLATION, Collator::ON);
    usort($input, function($a, $b) use ($c) {
      return sort_key_cmp($c, $a, $b);
    });
    var_dump(array($locale.' numeric' => $input));
  }
}
