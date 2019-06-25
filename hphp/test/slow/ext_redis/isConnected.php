<?hh <<__EntryPoint>> function main(): void {
$r = new \Redis();
var_dump($r->isConnected());
}
