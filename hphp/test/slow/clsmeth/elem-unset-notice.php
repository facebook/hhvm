<?hh

class Foo { static function bar() {} }
class P { function __construct(public mixed $m) {} }

function LV(mixed $m): mixed { return __hhvm_intrinsics\launder_value($m); }

function unset_static(): void {
  $cm = class_meth(Foo::class, 'bar');
  $am = varray[$cm];
  $dm = darray[0 => $cm];
  $om = new P($cm);

  unset($cm[1]);
  unset($cm[0]);
  unset($am[0][1]);
  unset($am[0][0]);
  unset($dm[0][1]);
  unset($dm[0][0]);
  unset($om->m[1]);
  unset($om->m[0]);

  var_dump($cm, $am, $dm, $om);
}

function unset_dynamic(): void {
  $cm = LV(class_meth(Foo::class, 'bar'));
  $am = LV(varray[$cm]);
  $dm = LV(darray[0 => $cm]);
  $om = LV(new P($cm));

  unset($cm[1]);
  unset($cm[0]);
  unset($am[0][1]);
  unset($am[0][0]);
  unset($dm[0][1]);
  unset($dm[0][0]);
  unset($om->m[1]);
  unset($om->m[0]);

  var_dump($cm, $am, $dm, $om);
}

function unset_inner() {
  $cm = LV(class_meth(Foo::class, 'bar'));

  // emptyish
  unset($cm[3][0]);

  var_dump($cm);

  // fatal
  unset($cm[0][0]);

  var_dump($cm);
}

function handle_error($_no, $msg, ...) {
  if ($msg === 'Implicit clsmeth to varray conversion' ||
      $msg === 'Implicit clsmeth to vec conversion') {
    echo "[NOTICE] $msg\n";
    return true;
  }
  return false;
}

<<__EntryPoint>>
function main() {
  set_error_handler(handle_error<>);

  unset_static();  unset_static();
  unset_dynamic(); unset_dynamic();

  unset_inner();
}
