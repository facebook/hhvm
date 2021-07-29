<?hh
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$foo = 'test';
$x = @$foo[6];

print @((int)$foo[100] + (int)$foo[130]);

print "\nDone\n";
}
