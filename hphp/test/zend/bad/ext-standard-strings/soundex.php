<?php

var_dump(soundex(""));
var_dump(soundex(-1));
var_dump(soundex(array()));

$array = array(
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
);

foreach ($array as $str) {
	var_dump(soundex($str));
}

echo "Done\n";
?>