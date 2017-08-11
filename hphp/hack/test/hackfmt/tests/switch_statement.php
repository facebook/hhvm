<?hh

switch ($foo->color) {
  case FooColor::RED:
    handleRedFoo($foo);
    break;
  case FooColor::GREEN:
  case FooColor::BLUE:
    handleGreenOrBlueFoo($foo);
    break;
  case FooColor::ORANGE:
    logOrangeFooEvent($foo);
    // FALLTHROUGH
  case FooColor::YELLOW:
    handleYellowishFoo($foo);
    break;
  default:
    handleOtherFoo($foo);
}
