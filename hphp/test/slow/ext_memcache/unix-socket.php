<?hh
<<__EntryPoint>> function main(): void {
$memcache = new Memcache();
$rc = $memcache->connect('unix:///run/memcached/memcached.sock', 0);
var_dump($rc);
}
