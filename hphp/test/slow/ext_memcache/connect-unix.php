<?hh
<<__EntryPoint>>
function main(): void {
  $m = new Memcache();
  $m->connect("unix:///tmp/hhvm-no-such-memcached.sock", 0);
  echo "ok\n";
}
