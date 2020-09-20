<?hh

function handle_error($_no, $msg, ...) {
  $pat = "/(Increment|Decrement) on string '(Foo|bar)'/";
  if (preg_match($pat, $msg)) {
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

  var_dump($cm[0] = 'Apple');
  var_dump($cm[1] = 'list');
  var_dump($am[0][1] = varray[1, 2, 3]);
  var_dump($am[0][0] = null);
  var_dump($dm[0][0] = 12);
  var_dump($dm[0][1] = 1.2);
  var_dump($om->m[1] = false);
  var_dump($om->m[0] = new stdclass);

  var_dump($cm, $am, $dm, $om);
}

function set_static2(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  var_dump($cm[] = 'XYZ');
  var_dump($am[0][] = varray[]);
  var_dump($dm[0][] = 99);
  var_dump($om->m[] = false);

  var_dump($cm, $am, $dm, $om);
}

function set_static3(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  var_dump($cm[0] .= '-ext1');
  var_dump($cm[1] .= '-xyz');
  var_dump($am[0][1] ^= '___');
  var_dump($am[0][0] = null);
  var_dump($dm[0][0] += 12);
  var_dump($dm[0][1] = 1.2);
  var_dump($om->m[1] *= 2);
  var_dump($om->m[0] /= 4);

  var_dump($cm, $am, $dm, $om);
}

function set_static4(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  var_dump($cm[0]++);
  var_dump($cm[1]++);
  var_dump($am[0][1]--);
  var_dump($am[0][0]++);
  var_dump($dm[0][0]--);
  var_dump($dm[0][1]--);
  var_dump($om->m[1]++);
  var_dump($om->m[0]--);

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic1(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  var_dump($cm[0] = 'Apple');
  var_dump($cm[1] = 'list');
  var_dump($am[0][1] = varray[1, 2, 3]);
  var_dump($am[0][0] = null);
  var_dump($dm[0][0] = 12);
  var_dump($dm[0][1] = 1.2);
  var_dump($om->m[1] = false);
  var_dump($om->m[0] = new stdclass);

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic2(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  var_dump($cm[] = 'XYZ');
  var_dump($am[0][] = varray[]);
  var_dump($dm[0][] = 99);
  var_dump($om->m[] = false);

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic3(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  var_dump($cm[0] .= '-ext1');
  var_dump($cm[1] .= '-xyz');
  var_dump($am[0][1] ^= '___');
  var_dump($am[0][0] = null);
  var_dump($dm[0][0] += 12);
  var_dump($dm[0][1] = 1.2);
  var_dump($om->m[1] *= 2);
  var_dump($om->m[0] /= 4);

  var_dump($cm, $am, $dm, $om);
}

function set_dynamic4(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  var_dump($cm[0]++);
  var_dump($cm[1]++);
  var_dump($am[0][1]--);
  var_dump($am[0][0]++);
  var_dump($dm[0][0]--);
  var_dump($dm[0][1]--);
  var_dump($om->m[1]++);
  var_dump($om->m[0]--);

  var_dump($cm, $am, $dm, $om);
}

<<__EntryPoint>>
function main() {
  set_error_handler('handle_error');

  set_static1();  set_static1();
  set_dynamic1(); set_dynamic1();
  set_static2();  set_static2();
  set_dynamic2(); set_dynamic2();
  set_static3();  set_static3();
  set_dynamic3(); set_dynamic3();
  set_static4();  set_static4();
  set_dynamic4(); set_dynamic4();
}
