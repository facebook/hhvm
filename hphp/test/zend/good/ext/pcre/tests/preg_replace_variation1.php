<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/
/*
* Testing preg_replace when the regex and the replacement are both arrays. */
<<__EntryPoint>> function main(): void {
$string = 'This is a string. It contains numbers (0-9) as well as parentheses and some other things!';
$new_string = preg_replace(vec['/\b\w{1}s/', '/(\d{1})-(\d{1})/', '/[\(!\)]/'], vec['test', '$1 to $2', '*'], $string);
print $new_string;
}
