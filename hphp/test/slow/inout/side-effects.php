<?hh

function ref(inout $x) :mixed{
}

function foo($r, inout $a, inout $b, $q, inout $c, ...$_) :mixed{
  $a = 'FOO:A';
  $b = 'FOO:B';
  $c = 'FOO:C';
  echo "foo()\n";
}

class Herp {
  public static $derp;
  public $baz;
}

function launder($x) :mixed{
  echo "launder()\n";
  return $x;
}
function get_arr() :mixed{
  $x = dict['alpha' => vec[dict['beta' => new Herp], null, new Herp, null]];
  $x['alpha'][0]['beta'] = dict['one' => '*BLANK*', 'two' => '*BLANK*'];
  return $x;
}

function main() :mixed{
  $x = get_arr();
  $i = 1;
  $t = 'alpha';
  foo(
    0,
    inout $x[launder('alpha')][--$i]['beta']['one'],
    inout $x[$t][++$i],
    0,
    inout $x[$t.launder('')][++$i],
    $i *= 2,
    $t = 'red',
  );
  var_dump($x);

  /* The following more general syntax is not allowed-
  $i = 0;
  $x = 'apple';
  Herp::$derp = get_arr();
  foo(
    $x = 'baz',
    inout (Herp::$derp[$t = launder('alpha')][$i++]['beta']->$x)['one'],
    inout $x,
    $a = 'TWO',
    inout Herp::$derp[$t][$i],
    $i *= 2,
    $old = Herp::$derp,
  );
  var_dump($x, Herp::$derp, $old);

  $i = 0;
  $x = 'apple';
  $Herp = Herp::class;
  $hh = 'Herp';
  Herp::$derp = get_arr();
  foo(
    $x = 'alpha',
    inout ${launder('hh')}::${launder('derp')}[$x][$i++],
    inout Herp::${launder('derp')}[$x][$i++],
    $a = 'TWO',
    inout ${$hh}::$derp[$x][$i++],
    $i *= 2,
    $x = 0,
  );
  var_dump($x, Herp::$derp);
  */

  $a = 'nope';
  $b = 'nope';
  $c = 'nope';
  foo(
    $a,
    inout $a,
    inout $b,
    $b,
    inout $c,
    $a,
    $c,
  );
  var_dump($a, $b, $c);

  $a = vec['yep'];
  $b = vec['nope'];
  $c = vec['yep'];
  foo(
    $a,
    inout $a[0],
    inout $b[0],
    $b,
    inout $c[0],
    ref(inout $a),
    ref(inout $c),
  );
  var_dump($a[0], $b[0], $c[0]);

  /* The following more general syntax is not allowed-
  $a = new stdClass;
  $a->x = 'one';
  $a->y = 'two';
  $saved = $a;
  foo(
    inout $a,
    inout $a->x,
    inout $a,
    $a,
    inout $a->y,
  );
  var_dump($saved, $a);

  Herp::$derp = 'foo';
  $a = 'Herp';
  foo(
    inout $a,
    inout $a::$derp,
    inout $a,
    $a,
    inout $a::$derp,
  );
  var_dump($a, Herp::$derp);

  $a = new stdClass;
  $a->x = 'one';
  $a->y = 'two';
  $saved = $a;
  foo(
    inout $a,
    inout $a,
    inout $a->x,
    $a,
    inout $a,
  );
  var_dump($saved, $a);

  $a = new stdClass;
  $a->x = 'one';
  $a->y = 'two';
  $saved = $a;
  foo(
    inout $a,
    inout $a->x,
    inout $a->y,
    $a = 'oops',
    inout $a,
  );
  var_dump($saved, $a);
  */
}


<<__EntryPoint>>
function main_side_effects() :mixed{
main();
}
