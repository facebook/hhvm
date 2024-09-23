<?hh

namespace N {
  function ns(): void {
    $s = shape('N\C' => "ns_str");
    $l = shape(C::class => "ns_lazy");
    $n = shape(nameof C => "ns_nameof");

    \var_dump($s['N\C']);
    \var_dump($s[C::class]);
    \var_dump($s[nameof C]);

    \var_dump($l['N\C']);
    \var_dump($l[C::class]);
    \var_dump($l[nameof C]);

    \var_dump($n['N\C']);
    \var_dump($n[C::class]);
    \var_dump($n[nameof C]);
  }

  function root(): void {
    $s = shape('C' => "root_str");
    $l = shape(\C::class => "root_lazy");
    $n = shape(nameof \C => "root_nameof");

    \var_dump($s['C']);
    \var_dump($s[\C::class]);
    \var_dump($s[nameof \C]);

    \var_dump($l['C']);
    \var_dump($l[\C::class]);
    \var_dump($l[nameof \C]);

    \var_dump($n['C']);
    \var_dump($n[\C::class]);
    \var_dump($n[nameof \C]);
  }
}

namespace {
  function x(): void {
    $s = shape('xhp_C' => "xhp_str");
    $l = shape(:C::class => "xhp_lazy");
    $n = shape(nameof :C => "xhp_nameof");

    var_dump($s['xhp_C']);
    var_dump($s[:C::class]);
    var_dump($s[nameof :C]);

    var_dump($l['xhp_C']);
    var_dump($l[:C::class]);
    var_dump($l[nameof :C]);

    var_dump($n['xhp_C']);
    var_dump($n[:C::class]);
    var_dump($n[nameof :C]);
  }

  function normal(): void {
    $s = shape('C' => "str");
    $l = shape(C::class => "lazyclass");
    $n = shape(nameof C => "nameof");

    var_dump($s['C']);
    var_dump($s[C::class]);
    var_dump($s[nameof C]);

    var_dump($l['C']);
    var_dump($l[C::class]);
    var_dump($l[nameof C]);

    var_dump($n['C']);
    var_dump($n[C::class]);
    var_dump($n[nameof C]);
  }

  <<__EntryPoint>>
  function main(): void {
    normal();
    x();
    N\ns();
    N\root();
  }
}
