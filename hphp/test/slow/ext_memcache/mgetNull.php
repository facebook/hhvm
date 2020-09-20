<?hh
<<__EntryPoint>> function main(): void {
$m = new Memcache();
$m->addServer('localhost', 11211);
$m->set('foo', '' );
var_dump( $m->get( 'foo' ) );
}
