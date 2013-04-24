<?php
/* Prototype  : array parse_ini_file(string filename [, bool process_sections])
 * Description: Parse configuration file 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */

echo "*** Testing parse_ini_file() : variation ***\n";
$output_file = __FILE__.".ini";
$iniFile = <<<FILE
[section1]
value1=original
value2=original
[section2]
value1=original
value2=different
FILE;

file_put_contents($output_file, $iniFile);

$a = parse_ini_file($output_file, true);
var_dump($a);
$a['section1']['value1'] = 'changed';
var_dump($a);

unlink($output_file);
?>
===DONE===