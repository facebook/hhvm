<?php

$algos = hash_algos();

foreach ($algos as $algo) {
	var_dump($algo);
	$orig = hash_init($algo);
	hash_update($orig, b"I can't remember anything");
	$copy = hash_copy($orig);
	var_dump(hash_final($orig));

	var_dump(hash_final($copy));
}

foreach ($algos as $algo) {
	var_dump($algo);
	$orig = hash_init($algo);
	hash_update($orig, b"I can't remember anything");
	$copy = hash_copy($orig);
	var_dump(hash_final($orig));

	hash_update($copy, b"Can’t tell if this is true or dream");
	var_dump(hash_final($copy));
}

echo "Done\n";
?>