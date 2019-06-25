<?hh
<<__EntryPoint>> function main(): void {
$db = new PDO( 'sqlite::memory:');

$db->sqliteCreateFunction('bar-alias', 'bar');
}
