<?hh
<<__EntryPoint>> function main(): void {
var_dump(strrchr("", ""));
var_dump(strrchr("abc", ""));
var_dump(strrchr("", "abc"));
var_dump(strrchr("abc", "abc"));
var_dump(strrchr("test ".chr(0)." test", " "));
var_dump(strrchr("test".chr(0)."string", "t"));

echo "Done\n";
}
