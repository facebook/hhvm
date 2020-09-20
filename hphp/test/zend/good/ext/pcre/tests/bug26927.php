<?hh <<__EntryPoint>> function main(): void {
$str = "a\000b";
$str_quoted = preg_quote($str);
var_dump(preg_match("!{$str_quoted}!", $str), $str_quoted);
}
