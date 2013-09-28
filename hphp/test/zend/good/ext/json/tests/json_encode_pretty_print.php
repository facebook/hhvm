<?php
function encode_decode($json) {
	$struct = json_decode($json);
	$pretty = json_encode($struct, JSON_PRETTY_PRINT);
	echo "$pretty\n";
	$pretty = json_decode($pretty);
	printf("Match: %d\n", $pretty == $struct);
}

encode_decode('[1,2,3,[1,2,3]]');
encode_decode('{"a":1,"b":[1,2],"c":{"d":42}}');
?>