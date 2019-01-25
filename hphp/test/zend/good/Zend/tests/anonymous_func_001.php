<?php 

for ($i = 0; $i < 10; $i++) {
	$a = create_function('', 'return '. $i .';');
	var_dump($a());

  // this is really not an implementation detail?!
	$b = "\0lambda_". ($i + 1);
	var_dump($b());
}

?>
