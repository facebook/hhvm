<?hh <<__EntryPoint>> function main(): void {
$iswin =  substr(PHP_OS, 0, 3) == "WIN";

if ($iswin) {
    $f = dirname(__FILE__) . '\\bug49847.tmp';
    $s = str_repeat(' ', 4097);
    $s .= '1';
    file_put_contents($f, $s);
    $output = null;
    $return_var = -1;
    exec('type ' . $f, inout $output, inout $return_var);
} else {
    $output = null;
    $return_var = -1;
    exec("printf %4098d 1", inout $output, inout $return_var);
}
var_dump($output);
if ($iswin) {
    unlink($f);
}
}
