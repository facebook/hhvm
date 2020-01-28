<?hh

include (__DIR__ . '/redis.inc');

<<__EntryPoint>>
function main_entry(): void {
  $r = NewRedisTestInstance();
  $r->setOption(Redis::OPT_PREFIX, GetTestKeyName(__FILE__) . ':');
  $r->setOption(Redis::OPT_SERIALIZER, Redis::SERIALIZER_PHP);

  var_dump(method_exists($r, '_serialize'));
  var_dump(method_exists($r, '_unserialize'));

  foreach (varray[null, true, false, 123, 456.0,
            "A string of words", varray[1,2,3]] as $val) {
    var_dump($r->_serialize($val));
    $r->set('A', $val);
    var_dump($r->get('A'));
  }

  $r->delete('A');
}
