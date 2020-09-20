<?hh
<<__EntryPoint>> function main(): void {
$str = (string)"a\000z";

var_dump(addslashes($str));
var_dump(addcslashes($str, (string)""));
var_dump(addcslashes($str, (string)"\000z"));
var_dump(addcslashes( $str, (string)"z"));

echo "Done\n";
}
