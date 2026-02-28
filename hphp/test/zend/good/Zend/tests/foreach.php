<?hh <<__EntryPoint>> function main(): void {
$foo = vec[1,2,3,4];
foreach($foo as $key => $val) {
    if($val == 3) {
        $foo[$key] = 0;
    } else {
        $foo[$key]++;
    }
}
var_dump($foo);
}
