<?php
$abc = "bar";
echo "foo\{$abc}baz\n";
echo "foo\{ $abc}baz\n";
echo <<<TEST
foo{$abc}baz
foo\{$abc}baz
foo\{ $abc}baz
TEST;
?>