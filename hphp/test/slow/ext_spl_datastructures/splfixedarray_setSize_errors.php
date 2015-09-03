<?php

$fixedarr = new SplFixedArray();

echo "Errors:", PHP_EOL;
$fixedarr->setSize([]);
$fixedarr->setSize("notanint");

echo "No Error:", PHP_EOL;
$fixedarr->setSize("5");
$fixedarr->setSize("6.6");
var_dump($fixedarr->getSize() == 6);
$fixedarr->setSize(2.2);
var_dump($fixedarr->getSize() == 2);
$fixedarr->setSize(true);
var_dump($fixedarr->getSize() == 1); // because php...
$fixedarr->setSize(false);
var_dump($fixedarr->getSize() == 0); // because php...
