<?hh

<<__Rx>>
function test_inout(inout bool $x) {
  $x = false;
}

class C {
  public $p;

  <<__Rx>>
  public function ok_base() {
    $a = array(1 => true);
    $x1 = $a[1]; // BaseL
    $x2 = (array(1 => true))[1]; // BaseC
    $x3 = __hhvm_intrinsics\launder_value(array(1 => true))[1]; // BaseR
    $x3 = $this->p; // BaseH
  }

  <<__Rx>>
  public function ok_dim() {
    $a = array(1 => array(2 => true), 'two' => array(2 => true));
    $p = new stdClass();
    $p->q = true;
    $o = new stdClass();
    $o->p = $p;
    $this->p = array(2 => true);
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

    // Define is valid on arrays
    $a[1][2] = false;
    $a['two'][2] = false;
    $a[$l1][2] = false;
    $a[__hhvm_intrinsics\launder_value(1)][2] = false;
    $a[][2] = false;

    // Define is also valid on $this
    $this->p[2] = false;
    $this->{$lp}[2] = false;
    $this->{__hhvm_intrinsics\launder_value('p')}[2] = false;

    // Unset is valid on arrays
    unset($a[1][2]);
    unset($a['two'][2]);
    unset($a[$l1][2]);
    unset($a[__hhvm_intrinsics\launder_value(1)][2]);

    // Unset is also valid on $this
    unset($this->p[2]);
    unset($this->{$lp}[2]);
    unset($this->{__hhvm_intrinsics\launder_value('p')}[2]);
  }

  <<__Rx>>
  public function bad_dim() {
    $o = new stdClass();
    $o->p = array(2 => true);
    $p = new stdClass();
    $p->q = array(3 => true);
    $this->p = $p;
    $lp = 'p';
    $lq = 'q';

    // Define on object that isn't $this
    $o->p[2] = false;
    $o->{$lp}[2] = false;
    $o->{__hhvm_intrinsics\launder_value('p')}[2] = false;

    // Second Define after dimming out of $this
    $this->p->q[3] = false;
    $this->p->{$lq}[3] = false;
    $this->p->{__hhvm_intrinsics\launder_value('q')}[3] = false;

    // Unset on object that isn't $this
    unset($o->p[2]);
    unset($o->{$lp}[2]);
    unset($o->{__hhvm_intrinsics\launder_value('p')}[2]);

    // Second Unset after dimming out of $this
    unset($this->p->q[3]);
    unset($this->p->{$lq}[3]);
    unset($this->p->{__hhvm_intrinsics\launder_value('q')}[3]);
  }

  <<__Rx>>
  public function ok_final() {
    $io1 = new stdClass();
    $io1->i = 1;
    $a = array('a' => array('i' => 1), 'o' => $io1);
    $o = new stdClass();
    $o->a = array('i' => 1);
    $io2 = new stdClass();
    $io2->i = 1;
    $o->o = $io2;
    $this->p = array('a' => array('i' => 1));

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

  <<__Rx>>
  public function bad_final() {
    $io = new stdClass();
    $io->i = 1;
    $a = array('o' => $io);

    // writes to object after Dim through array
    $a['o']->x = 2;
    $a['o']->x++;
    $a['o']->x *= 2;
    unset($a['o']->x);

    $this->p = new stdClass();
    // writes to object after Dim trough this
    $this->p->q = 2;
    $this->p->q++;
    $this->p->q *= 2;
    unset($this->p->q);

    $io2 = new stdClass();
    $io2->i = 1;
    $this->p = array(2 => $io2);
    // writes to object after Dim through this and then array
    $this->p[2]->q = 2;
    $this->p[2]->q++;
    $this->p[2]->q *= 2;
    unset($this->p[2]->q);
  }
}

<<__EntryPoint>>
function main() {
  $c = new C();
  $c->ok_base();
  $c->ok_dim();
  $c->bad_dim();
  $c->ok_final();
  $c->bad_final();
  echo "Done\n";
}
