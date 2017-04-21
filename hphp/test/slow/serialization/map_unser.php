<?hh
// Remember to also update map_unser_big, which exercises the fast path.

$data = [
  'K:6:"HH\\Map":0:{}',
  'K:6:"HH\\Map":1:{s:3:"bar";i:7;}',
  'K:6:"HH\\Map":1:{s:3:"bar";i:;}',
  'K:6:"HH\\Map":1:{s:3:"bar";i:-;}',
  'K:6:"HH\\Map":1:{s::"";i:;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;s:5:"extra";}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;s:5:"extra";i:456;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123',
  'K:6:"HH\\Map":2:{s:-3:"bar";i:3;s:5:"bling";i:123;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:55:"bling";i:123;}',
  'K:6:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;}blah',
  'K:7:"HH\\Map":2:{s:3:"bar";i:3;s:5:"bling";i:123;}'
];

foreach($data as $serialized) {
  var_dump(unserialize($serialized));
}

function makeBig($n) {
  $m = Map { 'foo' => 123, 'bar' => 9876543210 };
  for ($i = 0; $i < $n; $i += 1) {
    $m['key' . $i] = $i;
  }
  return $m;
}

$sizes = [ 0, 1, 100, 1000];

foreach ($sizes as $n) {
  var_dump(unserialize(serialize(makeBig($n))));
}
