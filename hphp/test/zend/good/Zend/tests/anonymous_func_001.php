<?php 

for ($i = 0; $i < 10; $i++) {
	$a = create_function('', 'return '. $i .';');
	var_dump($a());
	
	$b = "\0lambda_". ($i + 1);
	var_dump($b());
}

?>