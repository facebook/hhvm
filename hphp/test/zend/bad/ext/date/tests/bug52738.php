<?php
class di extends DateInterval {
    public $unit = 1;
}

$I = new di('P10D');
echo $I->unit."\n";
$I->unit++;
echo $I->unit."\n";
$I->unit = 42;
echo $I->unit."\n";
$I->d++;
print_r($I);