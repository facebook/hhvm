<?php

$heredoc = <<<	A

foo

	A;
A;

var_dump(strlen($heredoc) == 9);

?>