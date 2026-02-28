<?hh
const PREG_CACHE_SIZE = 4096+1;
<<__EntryPoint>> function main(): void {
$re = '';
$str = str_repeat('x', PREG_CACHE_SIZE);

for ($i=0; $i < PREG_CACHE_SIZE; ++$i) {
    $re .= '.';
    if (!preg_match("/$re/", $str)) {
        exit('non match. error');
    }
}

var_dump(preg_match('/./', $str));   // this one was already deleted from the cache
var_dump(preg_match("/$re/", $str)); // but not this one

echo "done\n";
}
