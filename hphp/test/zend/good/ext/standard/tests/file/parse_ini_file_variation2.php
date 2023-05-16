<?hh
/* Prototype  : array parse_ini_file(string filename [, bool process_sections])
 * Description: Parse configuration file 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing parse_ini_file() : variation ***\n";
$output_file = sys_get_temp_dir().'/'.'file.ini';
$iniFile = <<<FILE
[section1]
value1=on
value2=off
[section2]
value1=true
value2=false
[section3]
value1=yes
value2=no
[section4]
value1=null
value2=

[section5]
value1="on"
value2="off"
[section6]
value1="true"
value2="false"
[section7]
value1="yes"
value2="no"
[section8]
value1="null"
value2=""

FILE;

file_put_contents($output_file, $iniFile);

$a = parse_ini_file($output_file, true);
var_dump($a);
unlink($output_file);
echo "===DONE===\n";
}
