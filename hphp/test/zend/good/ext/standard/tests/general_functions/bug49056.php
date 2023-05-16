<?hh
<<__EntryPoint>> function main(): void {
$string = <<<EOT
Cooking_furniture="KÃ¼chen MÃ¶bel (en)"
KÃ¼chen_MÃ¶bel="Cooking furniture (en)"
EOT;

$filename = sys_get_temp_dir().'/'.'bug49056.tmp';

file_put_contents( $filename, $string);

var_dump(parse_ini_file($filename));

unlink($filename);
}
