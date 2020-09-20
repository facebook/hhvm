<?hh <<__EntryPoint>> function main(): void {
$mc = new Memcached;
$mc->setOption(Memcached::OPT_LIBKETAMA_COMPATIBLE, true);
var_dump($mc->getOption(Memcached::OPT_HASH) === Memcached::HASH_MD5);
var_dump($mc->getOption(Memcached::OPT_DISTRIBUTION) === Memcached::DISTRIBUTION_CONSISTENT_WEIGHTED);
$mc->setOption(Memcached::OPT_LIBKETAMA_COMPATIBLE, false);
var_dump($mc->getOption(Memcached::OPT_HASH) === Memcached::HASH_DEFAULT);
var_dump($mc->getOption(Memcached::OPT_DISTRIBUTION) === Memcached::DISTRIBUTION_MODULA);
var_dump(defined('Memcached::OPT_LIBKETAMA_HASH'));
}
