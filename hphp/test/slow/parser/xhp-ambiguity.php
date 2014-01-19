<?hh
function test() {
  /**
   * Rules for XHP names and guidelines for avoid ambiguities:
   *
   * (1) An XHP name may not have a space between the initial ':'
   *     and the rest of the name. For example, ": foo" will not
   *     be recognized as an XHP name by the parser.
   *
   * (2) An XHP name must always be preceded by whitespace if the
   *     previous token was ':'. Examples:
   *
   *       function bar(): :foo{}
   *                      ^
   *                      this space is needed
   *
   *       $x=$y?: :foo::bar();
   *              ^
   *              this space is needed
   *
   *       $x=$y?:(:foo::bar());
   *              ^
   *              wrapping the static method call in parentheses
   *              also works
   *
   * (3) An XHP name should be preceded by whitespace if it is not
   *     preceded by '(', '[', '{', or ',' and it is used as the base
   *     in a static method call, static property access, or class
   *     constant access. Note that when an XHP name is used in a
   *     function signature it's okay to use the XHP name without
   *     preceding whitespace as long as the previous token is not
   *     ':'. Examples:
   *
   *       $x=$y? :foo::bar():$z;
   *             ^
   *             this space is needed so that ":foo" is recognized
   *             as an XHP name
   *
   *       if(0){} :foo::bar();
   *              ^
   *              this space is needed so that ":foo" is recognized
   *              as an XHP name
   *
   *       $x=$y?(:foo::bar()):$z;
   *             ^
   *             wrapping the static method call in parentheses
   *             also works
   *
   *       function bar(:foo $x,?:foo $y): :foo{}
   *                    ^       ^         ^
   *                    no whitespace is needed when using XHP names
   *                    in function signatures unless the previous
   *                    token was ':'
   *
   *       function bar():?:foo{}
   *                      ^
   *                      no space is needed here
   *
   * (4) When ':' is preceded by whitespace, it should be followed
   *     by whitespace in order to avoid ambiguity with XHP names.
   *     Examples:
   *
   *       $x=$y?$z : foo();
   *                 ^
   *                 this space is needed to avoid ambiguity
   *                 with XHP names (because the ':' is preceded
   *                 by whitespace)
   *
   *       $x=$y?$z :(foo());
   *                 ^
   *                 wrapping the expression in parentheses also
   *                 works
   *
   *       $x=$y?:foo::bar();
   *              ^
   *              no whitespace is needed after the ':' to avoid
   *              ambiguity in this case (because there is no
   *              whitespace before the ':')
   *
   */
  $a = false?:array();
  $a = false?:floor(12.98);
  $a = b()?:NULL;

  $x=$y?:null;
  $x=$y?:foo();
  $x=$y?:FOO;
  $x=$y?:C::foo();
  $x=$y?:C::$x;
  $x=$y?:C::FOO;

  $x=$y?Vector{}:null;
  $x=$y?Vector{}:foo();
  $x=$y?Vector{}:FOO;
  $x=$y?Vector{}:C::foo();
  $x=$y?Vector{}:C::$x;
  $x=$y?Vector{}:C::FOO;

  $x=$y?$y : foo::bar();
  $x=$y?$y :foo::bar();
  $x=:foo::bar();
  label:foo::bar();
  $x=$y?: foo::bar();
  $x=$y? :foo::bar() : 0;
  $x=${'y'}? : foo();
  $x=${'y'} ?: foo();
  $x=${'y'}?: foo();
  $x=${'y'} ? : foo();
  $x=${'y'}? : foo;
  $x=${'y'}?: foo;
  $x=${'y'} ?: foo;
  $x=${'y'} ? : foo;
  $x=$y?$y:foo::bar();
  $x=$y?$y: foo::bar();
  $x=$y?$y :foo();
  $x=$y?$y :foo;
  $x=$y?$y :foo::bar();
  $x=$obj->{'foo'}?: foo::bar();
  $x=$obj->{'foo'}? : foo();
  $x=$obj->{'foo'}? : foo;
  $x=$obj->{'foo'}? :foo::bar() : 0;
  $x=$obj->{'foo'}? : foo::bar();
  $x=true?$obj->{'foo'}: foo::bar();
  $x=true?$obj->{'foo'} : foo::bar();
  $x=${'y'}?:foo();
  $x=$y?${'y'}:foo();
  $x=$y?$y:foo();
  $x=$y?($y):foo();
  if(0){} :foo::bar();
  function bar1(:foo $x): :foo {}
  function bar2(:foo $x):?:foo {}
  function bar3(:foo $x): ?:foo {}
  function bar4(?:foo $x): :foo {}
  function bar5(?:foo $x):?:foo {}
  function bar6(?:foo $x): ?:foo {}

  /**
   * Examples which violate rule #1
   */

  // var_dump(: foo::bar());

  /**
   * Examples which violate rule #2
   */

  // function bar7(?:foo $x)::foo{}
  // $x=$y?::foo::bar();

  /**
   * Examples which violate rule #3
   */

  // $x=$y?:foo::bar():$z;
  // $x=${'y'}?:foo::bar():$z;
  // $x=$y?:foo::bar():$z;
  // if(0){}:foo::bar();

  /**
   * Examples which violate rule #4
   */

  // $x=$y? :foo();
  // $x=$y? :FOO;
  // $x=$y? :foo::bar();
  // $x=$y? :foo::BAR;
  // $x=${'y'} ? :foo();
  // $x=${'y'} ? :FOO;
  // $x=${'y'} ? :foo::bar();
  // $x=${'y'} ? :foo::BAR;
  // $x=$obj->{'foo'}? :foo();
  // $x=$obj->{'foo'}? :FOO;
  // $x=$obj->{'foo'}? :foo::bar();
  // $x=$obj->{'foo'}? :foo::BAR;
  // $x=true?$obj->{'foo'} :foo();
  // $x=true?$obj->{'foo'} :FOO;
  // $x=true?$obj->{'foo'} :foo::bar();
  // $x=true?$obj->{'foo'} :foo::BAR;

}
echo "Done\n";
