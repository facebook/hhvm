<?hh
<<__EntryPoint>> function main(): void {
$str = <<< EOF
[section]
part1.*.part2 = 1
EOF;

$file = sys_get_temp_dir().'/'.'bug46347.ini';
file_put_contents($file, $str);

var_dump(parse_ini_file($file));

unlink($file);
}
