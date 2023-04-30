<?hh
<<__EntryPoint>> function main(): void {
$name = tempnam(sys_get_temp_dir(), 'readline.tmp');

readline_add_history('foo');
readline_add_history('');
readline_add_history('1');
readline_add_history('');
readline_write_history($name);

var_dump(file_get_contents($name));

unlink($name);
}
