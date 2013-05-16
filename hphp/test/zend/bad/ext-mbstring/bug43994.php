<?php
/* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
 * Description: Regular expression match for multibyte string 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * mb_ereg 'successfully' matching incorrectly: 
 * Bug now seems to be fixed - error message is now generated when an 'empty'
 * pattern is supplied to mb_ereg. Similar error message to ereg().
 */

$unset_var = 10;
unset ($unset_var);
$inputs = array(NULL, null, false, FALSE, "", '', @$undefined_var,
@$unset_var);

$iterator = 1;
foreach($inputs as $input) {
	if(@is_array($mb_regs)){
		$mb_regs = '';
	}
	echo "\n-- Iteration $iterator --\n";
	echo "Without \$regs arg:\n";
	var_dump( mb_ereg($input, 'hello, world') );
	echo "With \$regs arg:\n";
	var_dump(mb_ereg($input, 'hello, world', $mb_regs));
	var_dump($mb_regs);
	$iterator++;
};
?>
