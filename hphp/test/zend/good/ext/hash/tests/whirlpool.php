<?hh <<__EntryPoint>> function main(): void {
echo hash('whirlpool', ''), "\n";
$s='---qwertzuiopasdfghjklyxcvbnm------qwertzuiopasdfghjklyxcvbnm---';
echo hash('whirlpool', $s), "\n";
echo hash('whirlpool', str_repeat($s.'0', 1000)), "\n";
}
