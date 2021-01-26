<?hh

function test_inout(inout bool $x)[rx] {
  $x = false;
}

class C {
  public $p;

  public function ok_base()[rx] {
    $a = darray[1 => true];
    $x1 = $a[1]; // BaseL
    $x2 = (darray[1 => true])[1]; // BaseC
    $x3 = $this->p; // BaseH
  }

  public function ok_dim()[rx] {
    $a = darray[1 => darray[2 => true], 'two' => darray[2 => true]];
    $p = new stdClass();
    $p->q = true;
    $o = new stdClass();
    $o->p = $p;
    $mo = new stdClass();
    $mo->p = darray[2 => true];
    $this->p = darray[2 => true];
    $l1 = 1;
    $lp = 'p';

    // None
    $n1 = $a[1][2] ?? false;
    $n2 = $a['two'][2] ?? false;
    $n3 = $a[$l1][2] ?? false;
    $n4 = $a[__hhvm_intrinsics\launder_value(1)][2] ?? false;
    $n5 = $o->p->q ?? false;
    $n6 = $o?->p->q ?? false;
    $n7 = $o->{$lp}->q ?? false;
    $n8 = $o->{__hhvm_intrinsics\launder_value('p')}->q ?? false;
    $n9 = $this->p[2] ?? false;
    $na = $this?->p[2] ?? false;
    $nb = $this->{$lp}[2] ?? false;
    $nc = $this->{__hhvm_intrinsics\launder_value('p')}[2] ?? false;

    // Warn
    $w1 = $a[1][2] ?? false;
    $w2 = $a['two'][2] ?? false;
    $w3 = $a[$l1][2] ?? false;
    $w4 = $a[__hhvm_intrinsics\launder_value(1)][2] ?? false;
    $w5 = $o->p->q ?? false;
    $w6 = $o?->p->q ?? false;
    $w7 = $o->{$lp}->q ?? false;
    $w8 = $o->{__hhvm_intrinsics\launder_value('p')}->q ?? false;
    $w9 = $this->p[2] ?? false;
    $wa = $this?->p[2] ?? false;
    $wb = $this->{$lp}[2] ?? false;
    $wc = $this->{__hhvm_intrinsics\launder_value('p')}[2] ?? false;

    // InOut
    test_inout(inout $a[1][2]);
    test_inout(inout $a['two'][2]);
    test_inout(inout $a[$l1][2]);
    test_inout(inout $a[__hhvm_intrinsics\launder_value(1)][2]);

    // Define is valid on arrays, objects in locals, and $this
    $a[1][2] = false;
    $a['two'][2] = false;
    $a[$l1][2] = false;
    $a[__hhvm_intrinsics\launder_value(1)][2] = false;

    $mo->p[2] = false;
    $mo->{$lp}[2] = false;
    $mo->{__hhvm_intrinsics\launder_value('p')}[2] = false;
    $this->p[2] = false;
    $this->{$lp}[2] = false;
    $this->{__hhvm_intrinsics\launder_value('p')}[2] = false;

    // Unset is valid on arrays, objects in locals, and $this
    unset($a[1][2]);
    unset($a['two'][2]);
    unset($a[$l1][2]);
    unset($a[__hhvm_intrinsics\launder_value(1)][2]);
    unset($mo->p[2]);
    unset($mo->{$lp}[2]);
    unset($mo->{__hhvm_intrinsics\launder_value('p')}[2]);
    unset($this->p[2]);
    unset($this->{$lp}[2]);
    unset($this->{__hhvm_intrinsics\launder_value('p')}[2]);
  }

  public function ok_final()[rx] {
    $io1 = new stdClass();
    $io1->i = 1;
    $a = darray['a' => darray['i' => 1], 'o' => $io1];
    $o = new stdClass();
    $o->a = darray['i' => 1];
    $io2 = new stdClass();
    $io2->i = 1;
    $o->o = $io2;
    $this->p = darray['a' => darray['i' => 1]];

    // QueryM is always OK, before or after Dim, reading prop or elem
    $r1 = $a['a'];
    $r2 = $o->o;
    $r3 = $a['a']['i'];
    $r4 = $a['o']->i;
    $r5 = $o->a['i'];
    $r6 = $o->o->i;

    // Writes to array after Dim through array
    $a['a']['i'] = 2;
    $a['a']['i']++;
    $a['a']['i'] *= 2;
    unset($a['a']['i']);

    // Writes to array after Dim through local object
    $o->a['i'] = 2;
    $o->a['i']++;
    $o->a['i'] *= 2;
    unset($o->a['i']);

    // Writes to array after Dim through $this
    $this->p['a']['i'] = 2;
    $this->p['a']['i']++;
    $this->p['a']['i'] *= 2;
    unset($this->p['a']['i']);

    // Writes to array before Dim
    $a['x'] = 2;
    $a['x']++;
    $a['x'] *= 2;
    unset($a['x']);

    // Writes to object before Dim
    $o->x = 2;
    $o->x++;
    $o->x *= 2;
    unset($o->x);

    // Writes to $this before Dim
    $this->p = 2;
    $this->p++;
    $this->p *= 2;
    unset($this->p);
  }
}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->ok_base();
  $c->ok_dim();
  $c->ok_final();
  echo "Done\n";
}
