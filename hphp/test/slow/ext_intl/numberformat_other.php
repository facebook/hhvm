<?php

$locales = array(
  'en_US',
  'en_GB',
  'es_AR',
  'es_ES',
  'fr_FR',
  'de_DE',
  'he_IL',
);

$styles = array(
  NumberFormatter::DECIMAL,
  NumberFormatter::CURRENCY,
);

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
    var_dump($formatter->format(array()));
    var_dump($formatter->format(array(5)));
  }
}

$formatter = new NumberFormatter('en_GB', NumberFormatter::PATTERN_DECIMAL);
var_dump($formatter->format('123456'));
