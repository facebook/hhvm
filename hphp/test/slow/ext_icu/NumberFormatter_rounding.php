<?php
include (__DIR__ . '/NumberFormatter_rounding.inc');

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
