<?php

$xml = '<?xml version="1.0" encoding="UTF-8"?><response><t a="apple" b="banana">6</t><t>7</t><t>8</t></response>';
$sxml = simplexml_load_string($xml);
var_dump(count($sxml->t));
var_dump((string)$sxml->t[0]);
var_dump((string)$sxml->t[1]);
var_dump((string)$sxml->t[2]);
var_dump(count($sxml->t->bogus));
var_dump(count($sxml->t->attributes()));
foreach ($sxml->bogus as $v) {
}
