<?hh

function handle_error($_no, $msg, ...) {
  $pat = "/(Increment|Decrement) on string '(Foo|bar)'/";
  if (preg_match($pat, $msg)) {
    echo "[NOTICE] $msg\n";
    return true;
  }
  if ($msg === 'Implicit clsmeth to varray conversion' ||
      $msg === 'Implicit clsmeth to vec conversion') {
    echo "[NOTICE] $msg\n";
    return true;
  }
  return false;
}

class Foo { static function bar() {} }
class P { function __construct(public mixed $m) {} }

function LV(mixed $m): mixed { return __hhvm_intrinsics\launder_value($m); }

function set_static1(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  $cm[0] = 'Apple';
  $cm[1] = 'list';
  $am[0][1] = varray[1, 2, 3];
  $am[0][0] = null;
  $dm[0][0] = 12;
  $dm[0][1] = 1.2;
  $om->m[1] = false;
  $om->m[0] = new stdclass;

  var_dump($cm, $am, $dm, $om);
}

function set_static2(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  $cm[] = 'XYZ';
  $am[0][] = varray[];
  $dm[0][] = 99;
  $om->m[] = false;

  var_dump($cm, $am, $dm, $om);
}

function set_static3(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
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
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
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
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  $cm[0] = 'Apple';
  $cm[1] = 'list';
  $am[0][1] = varray[1, 2, 3];
  $am[0][0] = null;
  $dm[0][0] = 12;
  $dm[0][1] = 1.2;
  $om->m[1] = false;
  $om->m[0] = new stdclass;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic2(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  $cm[] = 'XYZ';
  $am[0][] = varray[];
  $dm[0][] = 99;
  $om->m[] = false;

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic3(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
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
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
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
function main() {
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
