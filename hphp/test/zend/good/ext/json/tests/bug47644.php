<?php

for ($i = 10000000000000000; $i < 10000000000000006; $i++) {
	var_dump(json_decode("[$i]"));
}


echo "Done\n";
?>