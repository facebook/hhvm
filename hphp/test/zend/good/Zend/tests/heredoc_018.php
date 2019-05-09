<?php
<<__EntryPoint>> function main() {
$heredoc = <<<	A

foo

	A;
A;

var_dump(strlen($heredoc) == 9);
}
