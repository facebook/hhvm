<?hh 
<<__EntryPoint>> function main(): void {
$arr = vec[new stdClass, 'stdClass'];

new $arr[0]();
new $arr[1]();

print "ok\n";
}
