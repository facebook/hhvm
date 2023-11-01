<?hh

// keep in sync with hphp/hack/test/tast/scope_resolution.php
function bar(): string {
  return 'class';
}
final class Foo {
  const string bar = 'baz';
  const vec<nothing> class = vec[];
  public static (function(): int) $_ = self::class<>;
  public static function bar(): float {
    return 3.14159;
  }
  public static function class(): int {
    return 42;
  }
}

// keep in sync with hphp/hack/test/tast/scope_resolution.php
function class_const(dynamic $x): void {
  Foo::bar;
  $x::bar;
  Foo::bar();
  $x::bar();

  (Foo::bar);
  (($x::bar));
  (((Foo::bar)))();
  (((($x::bar))))();

  Foo::bar\baz;
  (Foo::bar\baz);
  Foo::bar\baz();
  (Foo::bar\baz)();

  Foo::{bar};
  (Foo::{bar});
  Foo::{bar}();
  (Foo::{bar})();

  Foo::{(bar)};
  (Foo::{((bar))});
  Foo::{(((bar)))}();
  (Foo::{((((bar))))})();

  Foo::{bar\baz};
  (Foo::{bar\baz});
  Foo::{bar\baz}();
  (Foo::{bar\baz})();

  Foo::class;
  (Foo::class);
  Foo::class();
  (Foo::class)();

  Foo::{''};
  (Foo::{''});
  Foo::{''}();
  (Foo::{''})();

  Foo::{"bar"};
  (Foo::{'bar'});
  Foo::{'bar'}();
  (Foo::{"bar"})();

  Foo::{"bar\\baz"};
  (Foo::{'bar\\baz'});
  Foo::{'bar\\baz'}();
  (Foo::{"bar\\baz"})();

  Foo::{'class'};
  (Foo::{"class"});
  Foo::{"class"}();
  (Foo::{'class'})();

  Foo::{'$x'};
  (Foo::{'$x'});
  Foo::{'$x'}();
  (Foo::{'$x'})();

  Foo::{"\$x"};
  (Foo::{"\$x"});
  Foo::{"\$x"}();
  (Foo::{"\$x"})();

  Foo::{'$_'};
  (Foo::{'$_'});
  Foo::{'$_'}();
  (Foo::{'$_'})();

  Foo::{"\$_"};
  (Foo::{"\$_"});
  Foo::{"\$_"}();
  (Foo::{"\$_"})();

  Foo::{<<<EOD
such amaze$ $
EOD
  };
  (Foo::{<<<'EOD'
very $wow
EOD
  })();
}

// keep in sync with hphp/hack/test/tast/scope_resolution.php
function class_get(dynamic $x): void {
  Foo::$x;
  $x::$x;
  $x::$x();

  (Foo::$x);
  (($x::$x));
  (((Foo::$x)))();
  (((($x::$x))))();

  Foo::$_;
  (Foo::$_);
  (Foo::$_)();

  Foo::{$x};
  (Foo::{$x});
  Foo::{$x}();
  (Foo::{$x})();

  Foo::{$_};
  (Foo::{$_});
  Foo::{$_}();
  (Foo::{$_})();

  Foo::{bar()}();
  Foo::{vsprintf('%s', 'wow')}();
  Foo::{(string)(123 + 456)}();

  Foo::{'ba'."r"}();

  Foo::{"bar\\".'baz'}();

  Foo::{"$x"}();

  Foo::{"$_"}();

  Foo::{"ba$x"}();

  Foo::{"ba$_"}();

  Foo::{$x()}();

  Foo::{$_()}();

  Foo::{<<<EOD
$much doge
EOD
  }();
}

function invalid_exprs(dynamic $x): void {
  // Foo::'bar';
  // (Foo::"bar");
  // Foo::"bar"();
  // (Foo::'bar')();

  Foo::{bar()};
  (Foo::{bar().baz()});
  (Foo::{vsprintf('%s', 'wow')})();

  Foo::{'ba'."r"};
  (Foo::{'ba'."r"});
  (Foo::{"b".'ar'})();

  Foo::{"$x"};
  (Foo::{"$x"});
  (Foo::{"$x"})();

  Foo::{"$_"};
  (Foo::{"$_"});
  (Foo::{"$_"})();

  Foo::{"ba$x"};
  (Foo::{"ba$x"});
  (Foo::{"ba$x"})();

  Foo::{"ba$_"};
  (Foo::{"ba$_"});
  (Foo::{"ba$_"})();

  Foo::{$x()};
  (Foo::{$x()});
  (Foo::{$x()})();

  Foo::{$_()};
  (Foo::{$_()});
  (Foo::{$_()})();
}
