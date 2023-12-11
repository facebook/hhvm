<?hh

function handle_error($_no, $msg, ...) :mixed{
  if ($msg === 'Cannot use a scalar value as an array') {
    echo "[NOTICE] $msg\n";
    return true;
  }
  return false;
}

class Foo { static function bar() :mixed{} }
class P { function __construct(public mixed $m)[] {} }

function LV(mixed $m): mixed { return __hhvm_intrinsics\launder_value($m); }

function set_static1(): void {
  $cm = Foo::bar<>;
  $am = vec[$cm];
  $dm = dict[0 => $cm];
  $om = new P($cm);

  $cm[0] = 'Apple';
  $cm[1] = 'list';
  $am[0][1] = vec[1, 2, 3];
  $am[0][0] = null;
  $dm[0][0] = 12;
  $dm[0][1] = 1.2;
  $om->m[1] = false;
  $om->m[0] = new stdClass;

  var_dump($cm, $am, $dm, $om);
}

function set_static2(): void {
  $cm = Foo::bar<>;
  $am = vec[$cm];
  $dm = dict[0 => $cm];
  $om = new P($cm);

  $cm[] = 'XYZ';
  $am[0][] = vec[];
  $dm[0][] = 99;
  $om->m[] = false;

  var_dump($cm, $am, $dm, $om);
}

function set_static3(): void {
  $cm = Foo::bar<>;
  $am = vec[$cm];
  $dm = dict[0 => $cm];
  $om = new P($cm);

  $cm[0] .= '-ext1';
  $cm[1] .= '-xyz';
  $am[0][1] ^= '___';
  $am[0][0] = null;
  $dm[0][0] += 12;
  $dm[0][1] = 1.2;
  $om->m[1] *= 2;
  $om->m[0] /= 4;

  var_dump($cm, $am, $dm, $om);
}

function set_static4(): void {
  $cm = Foo::bar<>;
  $am = vec[$cm];
  $dm = dict[0 => $cm];
  $om = new P($cm);

  $cm[0]++;
  $cm[1]++;
  $am[0][1]--;
  $am[0][0]++;
  $dm[0][0]--;
  $dm[0][1]--;
  $om->m[1]++;
  $om->m[0]--;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic1(): void {
  $cm = LV(Foo::bar<>);
  $am = LV(vec[$cm]);
  $dm = LV(dict[0 => $cm]);
  $om = LV(new P($cm));

  $cm[0] = 'Apple';
  $cm[1] = 'list';
  $am[0][1] = vec[1, 2, 3];
  $am[0][0] = null;
  $dm[0][0] = 12;
  $dm[0][1] = 1.2;
  $om->m[1] = false;
  $om->m[0] = new stdClass;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic2(): void {
  $cm = LV(Foo::bar<>);
  $am = LV(vec[$cm]);
  $dm = LV(dict[0 => $cm]);
  $om = LV(new P($cm));

  $cm[] = 'XYZ';
  $am[0][] = vec[];
  $dm[0][] = 99;
  $om->m[] = false;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic3(): void {
  $cm = LV(Foo::bar<>);
  $am = LV(vec[$cm]);
  $dm = LV(dict[0 => $cm]);
  $om = LV(new P($cm));

  $cm[0] .= '-ext1';
  $cm[1] .= '-xyz';
  $am[0][1] ^= '___';
  $am[0][0] = null;
  $dm[0][0] += 12;
  $dm[0][1] = 1.2;
  $om->m[1] *= 2;
  $om->m[0] /= 4;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic4(): void {
  $cm = LV(Foo::bar<>);
  $am = LV(vec[$cm]);
  $dm = LV(dict[0 => $cm]);
  $om = LV(new P($cm));

  $cm[0]++;
  $cm[1]++;
  $am[0][1]--;
  $am[0][0]++;
  $dm[0][0]--;
  $dm[0][1]--;
  $om->m[1]++;
  $om->m[0]--;

  var_dump($cm, $am, $dm, $om);
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handle_error<>);

  set_static1();  set_static1();
  set_dynamic1(); set_dynamic1();
  set_static2();  set_static2();
  set_dynamic2(); set_dynamic2();
  set_static3();  set_static3();
  set_dynamic3(); set_dynamic3();
  set_static4();  set_static4();
  set_dynamic4(); set_dynamic4();
}
