<?hh

function main() {
  echo "=== Falsy ===\n";

  $v  = Vector {};
  $m  = Map {};
  $sm = StableMap {};
  $s  = Set {};

  var_dump((bool) $v);
  var_dump((bool) $m);
  var_dump((bool) $sm);
  var_dump((bool) $s);

  if ($v)  {throw new Exception('vector');}    else {echo "else\n";}
  if ($m)  {throw new Exception('map');}       else {echo "else\n";}
  if ($sm) {throw new Exception('stablemap');} else {echo "else\n";}
  if ($s)  {throw new Exception('set');}       else {echo "else\n";}

  echo "=== Reflection on Falsy ===\n";
  $rc_v  = new ReflectionClass($v);
  var_dump($rc_v->getName() === 'HH\Vector');
  $rc_m  = new ReflectionClass($m);
  var_dump($rc_m->getName() === 'Map');
  $rc_sm = new ReflectionClass($sm);
  var_dump($rc_sm->getName() === 'StableMap');
  $rc_s  = new ReflectionClass($s);
  var_dump($rc_s->getName() === 'HH\Set');

  $rm_v  = new ReflectionMethod($v, 'isEmpty');
  var_dump($rm_v->getName() === 'isEmpty');
  $rm_m  = new ReflectionMethod($m, 'isEmpty');
  var_dump($rm_m->getName() === 'isEmpty');
  $rm_sm = new ReflectionMethod($sm, 'isEmpty');
  var_dump($rm_sm->getName() === 'isEmpty');
  $rm_s  = new ReflectionMethod($s, 'isEmpty');
  var_dump($rm_s->getName() === 'isEmpty');

  echo "=== Truthy ===\n";

  $v[]       = 1;
  $sm['key'] = 1;
  $m['key']  = 1;
  $s[]       = 1;
  $p         = Pair {1, 2};

  var_dump((bool) $v);
  var_dump((bool) $m);
  var_dump((bool) $sm);
  var_dump((bool) $s);
  var_dump((bool) $p);

  if ($v)  {echo "if\n";} else {throw new Exception('vector');}
  if ($m)  {echo "if\n";} else {throw new Exception('map');}
  if ($sm) {echo "if\n";} else {throw new Exception('stablemap');}
  if ($s)  {echo "if\n";} else {throw new Exception('set');}
  if ($p)  {echo "if\n";} else {throw new Exception('pair');}
}

main();
