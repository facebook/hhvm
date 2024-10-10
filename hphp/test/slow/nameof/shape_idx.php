<?hh

namespace N {
  function ns(): void {
    $s = shape('N\C' => "ns_str");
    $l = shape(C::class => "ns_lazy");
    $n = shape(nameof C => "ns_nameof");

    \var_dump(Shapes::idx($s, 'N\C'));
    \var_dump(Shapes::idx($s, C::class));
    \var_dump(Shapes::idx($s, nameof C));

    \var_dump(Shapes::idx($l, 'N\C'));
    \var_dump(Shapes::idx($l, C::class));
    \var_dump(Shapes::idx($l, nameof C));

    \var_dump(Shapes::idx($n, 'N\C'));
    \var_dump(Shapes::idx($n, C::class));
    \var_dump(Shapes::idx($n, nameof C));
  }

  function root(): void {
    $s = shape('C' => "root_str");
    $l = shape(\C::class => "root_lazy");
    $n = shape(nameof \C => "root_nameof");

    \var_dump(Shapes::idx($s, 'C'));
    \var_dump(Shapes::idx($s, \C::class));
    \var_dump(Shapes::idx($s, nameof \C));

    \var_dump(Shapes::idx($l, 'C'));
    \var_dump(Shapes::idx($l, \C::class));
    \var_dump(Shapes::idx($l, nameof \C));

    \var_dump(Shapes::idx($n, 'C'));
    \var_dump(Shapes::idx($n, \C::class));
    \var_dump(Shapes::idx($n, nameof \C));
  }
}

namespace {
  function x(): void {
    $s = shape('xhp_C' => "xhp_str");
    $l = shape(:C::class => "xhp_lazy");
    $n = shape(nameof :C => "xhp_nameof");

    var_dump(Shapes::idx($s, 'xhp_C'));
    var_dump(Shapes::idx($s, :C::class));
    var_dump(Shapes::idx($s, nameof :C));

    var_dump(Shapes::idx($l, 'xhp_C'));
    var_dump(Shapes::idx($l, :C::class));
    var_dump(Shapes::idx($l, nameof :C));

    var_dump(Shapes::idx($n, 'xhp_C'));
    var_dump(Shapes::idx($n, :C::class));
    var_dump(Shapes::idx($n, nameof :C));
  }

  function normal(): void {
    $s = shape('C' => "str");
    $l = shape(C::class => "lazyclass");
    $n = shape(nameof C => "nameof");

    var_dump(Shapes::idx($s, 'C'));
    var_dump(Shapes::idx($s, C::class));
    var_dump(Shapes::idx($s, nameof C));

    var_dump(Shapes::idx($l, 'C'));
    var_dump(Shapes::idx($l, C::class));
    var_dump(Shapes::idx($l, nameof C));

    var_dump(Shapes::idx($n, 'C'));
    var_dump(Shapes::idx($n, C::class));
    var_dump(Shapes::idx($n, nameof C));
  }

  <<__EntryPoint>>
  function main(): void {
    normal();
    x();
    N\ns();
    N\root();
  }
}
