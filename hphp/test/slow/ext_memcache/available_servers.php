<?hh
<<__EntryPoint>> function main(): void {
// 1. nonexisting connect.
$m = new Memcache();
var_dump($m->connect('nonexistinghost', 123));
var_dump($m->set('foo', 'bar'));
var_dump($m->get('foo'));

// 2. nonexistinghost add server.
$m = new Memcache();
var_dump($m->addServer('nonexistinghost', 123));
var_dump($m->set('foo', 'bar'));
var_dump($m->get('foo'));
var_dump($m->delete('foo'));

// 3. existing add server.
$m = new Memcache();
var_dump($m->addServer('localhost', 11211));
var_dump($m->set('foo', 'bar'));
var_dump($m->set('baz', 'qux'));
var_dump($m->get('foo'));
var_dump($m->get(vec['foo', 'baz']));
}
