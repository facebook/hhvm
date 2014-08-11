<?php
$codes = array(
	"<?php __halt_compiler",
	"<?php __halt_compiler(",
	"<?php __halt_compiler();",
	"<?php __halt_compiler();ABC",
	"<?php __halt_compiler\n(\n)\n;ABC",
	"<?php __halt_compiler\nabc\ndef\nghi ABC",
);
foreach ($codes as $code) {
	$tokens = token_get_all($code);
	var_dump($tokens);
	
	$code = '';
	foreach ($tokens as $t)
	{
		$code .= isset($t[1]) ? $t[1] : $t;
	}
	var_dump($code);
}

?>
