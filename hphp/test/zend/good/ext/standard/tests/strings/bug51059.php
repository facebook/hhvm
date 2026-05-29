<?hh <<__EntryPoint>> function main(): void {
$res = crypt('a', '_');
if ($res === '*0' || $res === '*1') echo 'OK';
else echo 'Not OK';
}
