<?hh


<<__EntryPoint>>
function main_1665() :mixed{
$xml = '<?xml version="1.0" encoding="UTF-8"?><response><t a="apple" b="banana">6</t><t>7</t><t>8</t></response>';
$sxml = simplexml_load_string($xml);
var_dump(count($sxml->t));
var_dump((string)$sxml->t->offsetGet(0));
var_dump((string)$sxml->t->offsetGet(1));
var_dump((string)$sxml->t->offsetGet(2));
var_dump(count($sxml->t->bogus));
var_dump(count($sxml->t->attributes()));
foreach ($sxml->bogus as $v) {
}
}
