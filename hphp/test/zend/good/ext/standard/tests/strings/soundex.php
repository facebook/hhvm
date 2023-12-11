<?hh
<<__EntryPoint>> function main(): void {
var_dump(soundex(""));
var_dump(soundex('-1'));
try { var_dump(soundex(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$array = vec[
"From",
"that",
"time",
"on",
"Sam",
"thought",
"that",
"he",
"sensed",
"a",
"change",
"in",
"Gollum",
"again.",
"He was more fawning and would-be friendly; but Sam surprised some strange looks in his eyes at times, especially towards Frodo."
];

foreach ($array as $str) {
	var_dump(soundex($str));
}

echo "Done\n";
}
