<?hh
<<__EntryPoint>> function main(): void {
$pdo = new PDO('sqlite::memory:');
var_dump($pdo->getAttribute(PDO::ATTR_SERVER_VERSION));
var_dump($pdo->getAttribute(PDO::ATTR_CLIENT_VERSION));
}
