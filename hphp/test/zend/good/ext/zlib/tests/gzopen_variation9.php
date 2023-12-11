<?hh
/* Prototype  : resource gzopen(string filename, string mode [, int use_include_path])
 * Description: Open a .gz-file and return a .gz-file pointer 
 * Source code: ext/zlib/zlib.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gzopen() : variation ***\n";

$modes = vec['r+', 'rf', 'w+' , 'e'];

$file = dirname(__FILE__)."/004.txt.gz";

foreach ($modes as $mode) {
    echo "mode=$mode\n";
    $h = gzopen($file, $mode);
    echo "gzopen=";
    var_dump($h);
    if ($h !== false) {
       gzclose($h);
    }
    echo "\n";
}
echo "===DONE===\n";
}
