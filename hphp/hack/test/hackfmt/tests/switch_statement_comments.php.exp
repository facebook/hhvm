<?hh

switch ($foo->color) {
  # Red foos handled here
  case FooColor::RED:
    // Red foos are best foos
    handleRedFoo($foo);
    break;
  // Green and blue foos are good too
  case FooColor::GREEN:
  case FooColor::BLUE:
    handleGreenOrBlueFoo($foo);
    break; // no fallthrough
  case FooColor::ORANGE:
    logOrangeFooEvent($foo);
    // We fall through here to also handle this as a yellowish foo.
    // FALLTHROUGH
  /* Yellow and yellowish foos handled here */
  case FooColor::YELLOW:
    handleYellowishFoo($foo);
    break;

  default:
    handleOtherFoo($foo);
}
