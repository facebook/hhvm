<?php
include (__DIR__ . '/../NumberFormatter_rounding.inc');

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
