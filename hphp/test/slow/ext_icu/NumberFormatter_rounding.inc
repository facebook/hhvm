<?hh
function test_rounding($locale, $digits, $mode = null) {
  $formatter = NumberFormatter::create($locale, NumberFormatter::DEFAULT_STYLE);
  $formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, $digits);
  if ($mode !== null) {
    $formatter->setAttribute(NumberFormatter::ROUNDING_MODE, $mode);
  }

  $values = vec[
    1.23,
    1.2327,
    1.2372,
    1.235,
    -1.23,
    -1.2327,
    -1.2372,
    -1.235
  ];

  foreach ($values as $value) {
    echo $value;
    echo " -> ";
    echo $formatter->format($value);
    echo "\n";
  }
}
