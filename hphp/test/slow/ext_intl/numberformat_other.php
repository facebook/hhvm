<?hh

function normalize_number_format(mixed $value): mixed {
  if ($value is string) {
    return str_replace(vec["\u{200f}", "$\u{00a0}"], vec['', '$'], $value);
  }
  return $value;
}

<<__EntryPoint>>
function main_numberformat_other() :mixed{
$locales = vec[
  'en_US',
  'en_GB',
  'es_AR',
  'es_ES',
  'fr_FR',
  'de_DE',
  'he_IL',
];

$styles = vec[
  NumberFormatter::DECIMAL,
  NumberFormatter::CURRENCY,
];

foreach ($locales as $locale) {
  foreach ($styles as $style) {
    echo "$locale -- $style\n";
    $formatter = new NumberFormatter($locale, $style);
    foreach (vec[100, 100.00, '100', 'not a number', true, false, vec[], vec[5]] as $value) {
      var_dump(normalize_number_format($formatter->format($value)));
    }
  }
}

$formatter = new NumberFormatter('en_GB', NumberFormatter::PATTERN_DECIMAL);
var_dump($formatter->format('123456'));
}
