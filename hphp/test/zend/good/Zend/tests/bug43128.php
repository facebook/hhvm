<?hh
<<__EntryPoint>> function main(): void {
$a = str_repeat("a", 10 * 1024 * 1024);

eval("class $a {}");

# call_user_func($a); // Warning
# $a->$a();           // Fatal error

new $a;                // Segmentation fault
echo "ok\n";
}
