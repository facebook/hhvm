<?hh


<<__EntryPoint>>
function main_numberformat_other() :mixed{
$locales = varray[
  'en_US',
  'en_GB',
  'es_AR',
  'es_ES',
  'fr_FR',
  'de_DE',
  'he_IL',
];

$styles = varray[
  NumberFormatter::DECIMAL,
  NumberFormatter::CURRENCY,
];

foreach ($locales as $locale) {
  foreach ($styles as $style) {
    echo "$locale -- $style\n";
    $formatter = new NumberFormatter($locale, $style);
    var_dump($formatter->format(100));
    var_dump($formatter->format(100.00));
    var_dump($formatter->format('100'));
    var_dump($formatter->format('not a number'));
    var_dump($formatter->format(true));
    var_dump($formatter->format(false));
    var_dump($formatter->format(varray[]));
    var_dump($formatter->format(varray[5]));
  }
}

$formatter = new NumberFormatter('en_GB', NumberFormatter::PATTERN_DECIMAL);
var_dump($formatter->format('123456'));
}
