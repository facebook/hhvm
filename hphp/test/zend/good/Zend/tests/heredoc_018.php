<?hh
<<__EntryPoint>> function main(): void {
$heredoc = <<<	A

foo

	A;
A;

var_dump(strlen($heredoc) == 9);
}
