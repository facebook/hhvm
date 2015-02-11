<?php
$formatter = NumberFormatter::create("en_US", NumberFormatter::DEFAULT_STYLE);
$formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, 2);

echo $formatter->format(1.23);
echo "\n";
echo $formatter->format("10.345");
echo "\n";

$formatter = NumberFormatter::create("en_US", NumberFormatter::DEFAULT_STYLE);
$formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, 0);
$formatter->setAttribute(NumberFormatter::ROUNDING_MODE,
                         NumberFormatter::ROUND_HALFUP);
echo $formatter->format(123450.67);
echo "\n";
echo $formatter->format("123456788.89");
echo "\n";
