<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_match('', ''));
var_dump(preg_match('      ', ''));
var_dump(preg_match('@@', ''));
var_dump(preg_match('12', ''));
var_dump(preg_match('<>', ''));
var_dump(preg_match('~a', ''));
var_dump(preg_match('@\@\@@', '@@'));
var_dump(preg_match('//z', '@@'));
var_dump(preg_match('{', ''));
}
