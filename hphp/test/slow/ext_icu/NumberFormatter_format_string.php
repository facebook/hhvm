<?php
$formatter = NumberFormatter::create("en-US", NumberFormatter::DEFAULT_STYLE);
$formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, 2);

echo $formatter->format(60.65);
echo "\n";
echo $formatter->format("60.65");
echo "\n";


$formatter->setAttribute(NumberFormatter::FRACTION_DIGITS, 0);
$formatter->setAttribute(NumberFormatter::ROUNDING_MODE,
                         NumberFormatter::ROUND_HALFUP);
echo $formatter->format(60.65);
echo "\n";
echo $formatter->format("60.65");
echo "\n";
