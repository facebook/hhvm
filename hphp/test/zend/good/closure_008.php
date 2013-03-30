<?php

function replace_spaces($text) {
	$lambda = function ($matches) {
		return str_replace(' ', '&nbsp;', $matches[1]).' ';
	};
	return preg_replace_callback('/( +) /', $lambda, $text);
}

echo replace_spaces("1 2 3\n");
echo replace_spaces("1  2  3\n");
echo replace_spaces("1   2   3\n");
echo "Done\n";
?>