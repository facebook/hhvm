<?hh

doThing(
  $foo ==> {
    switch ($foo->color) {
      case FooType::RED:
        handleRedFoo($foo);
        break;
      case FooType::GREEN:
      case FooType::BLUE:
      case FooType::VERY_LONG_COLOR_NAME_WHICH_WILL_CAUSE_A_LINE_BREAK_WITHIN_THIS_LABEL:
        handleNonRedFoo($foo);
        break;
    }
  }
);
