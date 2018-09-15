<?hh // strict

/** These are in spec and also supported by hphpc. When testing qualified names,
  * make sure to test both with and without leading namespaces.
  */
function test_valid_cases(dynamic $y): void {
  // qualified-name
  $x = $y instanceof \Foo; // qualified-name
  $x = $y instanceof Foo; // qualified-name

  // new-variable
  $x = $y instanceof $variable; // simple-variable
  $x = $y instanceof $y->variable[0]; // new-variable [ expression opt ]
  $x = $y instanceof $y->variable{"string"}; // new-variable { expression }
  $x = $y instanceof $y->variable; // new-variable -> member-name
  $x = $y instanceof $y->$variable; // new-variable -> member-name
  $x = $y instanceof $y->{"string"}; // new-variable -> member-name
  $x = $y instanceof \Foo::$variable; // qualified-name :: simple-variable
  $x = $y instanceof Foo::$variable; // qualified-name :: simple-variable
  $x = $y instanceof static::$variable; // relative-scope :: simple-variable
  $x = $y instanceof self::$variable; // relative-scope :: simple-variable
  $x = $y instanceof parent::$variable; // relative-scope :: simple-variable
  $x = $y instanceof $y::$y; // new-variable :: simple-variable
  $x = $y instanceof \Foo::$y::$y; // new-variable :: simple-variable
  $x = $y instanceof Foo::$y::$y; // new-variable :: simple-variable

  $x = $y instanceof \Foo::$y[$member_name]::$y; // scope resolution after brackets
  $x = $y instanceof Foo::$y[$member_name]::$y; // scope resolution after brackets
  $x = $y instanceof \Foo::$y{$member_name}::$y; // scope resolution after braces
  $x = $y instanceof Foo::$y{$member_name}::$y; // scope resolution after braces

  // Hack-specific
  $x = $y instanceof :xhp; // XHP names are qualified-names
  $x = $y instanceof :xhp :: $scope; // qualified-name :: simple-variable

  // Allow expressions in parens
  $x = $y instanceof (get_class("Foo"));
}


// These are all errors (should be 13 of them, although some lines spawn more
// than one error so the actual count will be 14+).

/** These are in spec but not supported by hphpc. */
function test_hphpc_spec_errors(dynamic $y): void {
  // hphpc doesn't support a scope resolution operator after a member access
  $z = $y instanceof \Foo::$y->$member_name::$y;

  // hphpc doesn't support empty brackets
  $x = $y instanceof $y->variable[];
}

/** These aren't even valid in the spec. */
function test_fully_invalid(dynamic $y): void {
  $x = $y instanceof 1;
  $x = $y instanceof 'foo';
  $x = $y instanceof ($z.$zz);
  $x = $y instanceof Foo::class;
  $x = $y instanceof (Foo::class);
  $x = $y instanceof (Foo);
  $x = $y instanceof $y(Foo);

  // This one looks like it should be allowed, but the spec doesn't allow nested
  // qualified names.
  $x = $y instanceof Foo::Bar::$variable;

  $x = $y instanceof Foo->$variable;
  $x = $y instanceof Foo->[$variable];
  $x = $y instanceof Foo->{$variable};
}
