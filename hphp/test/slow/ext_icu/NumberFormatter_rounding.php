<?php
function test_rounding($locale, $digits, $mode = null) {
  $formatter = NumberFormatter::create($locale, NumberFormatter::DEFAULT_STYLE);
  $formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, $digits);
  if ($mode !== null) {
    $formatter->setAttribute(NumberFormatter::ROUNDING_MODE, $mode);
  }

  $values = array(
    1.23,
    1.2327,
    1.2372,
    1.235,
    -1.23,
    -1.2327,
    -1.2372,
    -1.235
  );

  foreach ($values as $value) {
    echo $value;
    echo " -> ";
    $to_print = $formatter->format($value);

    //
    // Remove the Unicode Character 'LEFT-TO-RIGHT MARK' (U+200E)
    // which some versions of ICU use for Farsi.
    // See http://www.fileformat.info/info/unicode/char/200e/index.htm
    //
    $to_print = str_replace("\xe2\x80\x8e", "", $to_print);

    //
    // Replace the Unicode Character 'MINUS SIGN' (U+2212)
    // which some versions of ICU use for Farsi.
    // See http://www.fileformat.info/info/unicode/char/2212/index.htm
    //
    $to_print = str_replace("\xe2\x88\x92", "-", $to_print);

    echo $to_print;
    echo "\n";
  }
}

echo "Testing en_US, default rounding mode, 3 digits\n";
test_rounding("en_US", 3);

echo "Testing en_US, rounding mode down, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_DOWN); // aka TRUNCATE

echo "Testing en_US, rounding mode up, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_UP);

echo "Testing en_US, rounding mode ceiling, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_CEILING);

echo "Testing en_US, rounding mode floor, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_FLOOR);

echo "Testing en_US, rounding mode half-down, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_HALFDOWN);

echo "Testing en_US, rounding mode half-up, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_HALFUP); // aka ROUND

echo "Testing en_US, rounding mode half-even, 2 digits\n";
test_rounding("en_US", 2, NumberFormatter::ROUND_HALFEVEN);

// TODO(t5921532) Add tests for ar_AE once icu has been updated to 53.1 or newer

echo "Testing fa_IR, default rounding mode, 3 digits\n";
test_rounding("fa_IR", 3);

echo "Testing fa_IR, rounding mode down, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_DOWN); // aka TRUNCATE

echo "Testing fa_IR, rounding mode up, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_UP);

echo "Testing fa_IR, rounding mode ceiling, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_CEILING);

echo "Testing fa_IR, rounding mode floor, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_FLOOR);

echo "Testing fa_IR, rounding mode half-down, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_HALFDOWN);

echo "Testing fa_IR, rounding mode half-up, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_HALFUP); // aka ROUND

echo "Testing fa_IR, rounding mode half-even, 2 digits\n";
test_rounding("fa_IR", 2, NumberFormatter::ROUND_HALFEVEN);
