<?hh // strict

namespace NS_nullsafe_member_selection;

// =================== use ?-> with an instance method =====================

class Bar1 {
  public function baz1(): int {
     return 5;
  }
  public static function sbaz1(): int {
    return 55;
  }
}

function get_Bar1(): ?Bar1 {
  if (rand(0, 10) === 5) {
    return null;
  }
  return new Bar1();
}

function foo1(): ?int {
  $b = get_Bar1();
  // Use the null-safe operator to access a method of class.

//  return $b?->sbaz1();	// error: method can't be static

  return $b?->baz1();
}

// =================== use ?-> with a property =====================

class Bar2 {
  public function __construct(public string $str = "") {}
  public static string $sstr = "xx";
}

function get_Bar2(): ?Bar2 {
  if (rand(0, 10) < 5) {
    return null;
  }
  return new Bar2("Hello");
}

function foo2(): ?string {
  $b = get_Bar2();
  // Use the null-safe operator to access a proprety of a class.

//  $b?->str = 'abc';	// error: ?-> syntax is not supported for lvalues

//  return $b?->sstr;	// error: property can't be static

  return $b?->str;
}

function main(): void {
  var_dump(foo1());
  var_dump(foo2());
}

/* HH_FIXME[1002] call to main in strict*/
main();
