<?hh
/* Prototype  : proto array array_count_values(array input)
 * Description: Return the value as key and the frequency of that value in input as value
 * Source code: ext/standard/array.c
 * Alias to functions:
 */

/*
 * Test behaviour with parameter variations
 */

class A {
    static function hello() :mixed{
      echo "Hello\n";
    }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_count_values() : parameter variations ***\n";
chdir(sys_get_temp_dir());
$ob = new A();

$fp = fopen("array_count_file", "w+");

$arrays = dict["bobk" => "bobv", 0 => "val", 6 => "val6", 7 => $fp, 8 => $ob];

var_dump (@array_count_values ($arrays));
echo "\n";


echo "Done";

unlink("array_count_file");
}
