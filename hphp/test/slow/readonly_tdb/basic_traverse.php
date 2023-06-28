<?hh
namespace ROTB\BasicTraverse;
class DisposeMe {
  public function __dispose() :mixed{}
}

class :el {
  attribute string attr;
}

class Foo {
  public int $x = 0;
}

class Bar {
  public function foo() :mixed{
    $_ = ro();
  }
  public static function bar() :mixed{
    $_ = ro();
  }
  public function __construct(public mixed $m, public mixed ...$rest) {}
}

function mixed_id(mixed $x): mixed {
  return () ==> {
    $_ = ro();
    return $x;
  };
}

function ro(): readonly Foo {
  return new Foo();
}

// <<__EntryPoint>>
function main(): void {
  // expressions
  $_ = readonly (() ==> {
    () ==> { $_ = ro(); return 1;}();
  })();
  $_ = - () ==> { $_ = ro(); return 1;}();

  $_ = () ==> { $_ = ro(); return true; }()
    ? () ==> { $_ = ro(); return null; }()
    : () ==> { $_ = ro(); return null; }();
  $_ =
    () ==> { $_ = ro(); return new Foo(); }()
  as Foo as? Foo is Foo;
  $foo = new Foo();
  darray[
    1 => () ==> { $_ = ro(); return null; }(),
  ];
  dict[
    1 => () ==> { $_ = ro(); return null; }(),
  ];
  varray[
    function() {$_ = ro(); return null; }(),
    () ==> { $_ = ro(); return null; }(),
  ];
  vec[
    () ==> { $_ = ro(); return null; }(),
  ];
  shape(
    'a' => () ==> {
      $_ = ro();
      return null;
    }(),
  );
  mixed_id(
    () ==> { $_ = ro(); return null; }(),
  );
  new Bar(
    () ==> { $_ = ro(); return null; }(),
    ...vec[
      () ==> { $_ = ro(); return null; }(),
    ]
  );
  $_ = () ==> {
    $_ = ro();
    return new Bar();
  }()->foo();
  $_ = Pair{
      () ==> { $_ = ro(); return null; }(),
      () ==> { $_ = ro(); return null; }(),
  };
  $_ = () ==> { $_ = ro(); return 1; }()
    |> $$ +
      () ==> { $_ = ro(); return 1; }();
  $_ = () ==> {
    $_ = ro(); return true; }()
    ? () ==> { $_ = ro(); return 1; }()
    : 2;
  $_ = false
    ? 2
    : () ==> { $_ = ro(); return 1; }();
  $_ = 1 + (int)((() ==> {
    $_ = ro();
    return 1;
  })());
  $v = vec[0];
  $_ = "{$v[() ==> {$_ = ro(); return 0;}()]}";

  // statements
  try {
    throw () ==> { $_ = ro(); return new Exception(''); }();
  } catch (Exception $_) {
      () ==> { $_ = ro();}();
  } finally {
      () ==> { $_ = ro();}();
  }
  $_ = () ==> {
    return () ==> {
      $_ = ro();
      return 3;
    };
  }();
  $_ = async () ==> {
    concurrent {
      $_ = await () ==> {
        $_ = ro();
      }();
      $_ = await () ==> {
        $_ = ro();
      }();
    } ;
  }();
  if (
    () ==> { $_ = ro(); return true;}()
  ) {}
  do {
    $_ = ro();
  } while (
    () ==> { $_ = ro(); return false; }()
  );
  while (
    () ==> { $_ = ro(); return false;}()
  ) {
    $_ = ro();
  }
  using (new DisposeMe()) {
    $_ = ro();
  }
  using (
    () ==> { $_ = ro(); return new DisposeMe(); }()
  );
  for (
    $_ = ro(),
    $i = 0;
    () ==> {
      $_ = ro();
      return $i < 2;
    }();
    $i++
  ) {
    $_ = ro();
  }
  switch(
    () ==> { $_ = ro(); return 3; }()
  ) {
    case (
      () ==> { $_ = ro(); return 1; }()
     ):
        $_ = ro();
    case (
      () ==> { $_ = ro(); return 3; }()
     ):
        $_ = ro();
  }

  // unstable features
  ExampleDsl`() ==> {
    $x = ${() ==> {$_ = ro(); return 1;}()};
  }`;

  $_ = <el attr={() ==> {$_ = ro(); return "h";}()} />;
}
