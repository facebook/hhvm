<?hh
const key = 'incr_decr_test';

//increment with initial value only works with binary protocol
const non_existant_key = 'incr_decr_test_fail';
<<__EntryPoint>>
function main_entry(): void {
  $mc = new Memcached;
  $mc->addServer('127.0.0.1', 11211);
  $mc->set(key, 0);
  var_dump($mc->get(key));
  $mc->increment(key, 3);
  var_dump($mc->get( key));
  $mc->decrement(key, 1);
  var_dump($mc->get(key));

  $mc2 = new Memcached;
  $mc2->setOption(Memcached::OPT_BINARY_PROTOCOL, true);
  $mc2->addServer('127.0.0.1', 11211);

  var_dump($mc2->increment(non_existant_key, 3));
  var_dump($mc2->getMulti(vec[non_existant_key]));
  var_dump($mc2->decrement(non_existant_key, 1));
  var_dump($mc2->getMulti(vec[non_existant_key]));

  // There is an issue with the return value from this section, especially as it
  // changes when memcached isn't clean - even with the delete below.
  $mc2->increment(non_existant_key, 3, 1);
  $result = $mc2->getMulti(vec[non_existant_key]);
  var_dump($result[non_existant_key]);

  // Cleanup
  $mc2->delete(non_existant_key);
}
