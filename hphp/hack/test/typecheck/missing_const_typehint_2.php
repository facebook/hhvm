<?hh // strict
/* HH_FIXME[1002] */
define('FOO', 123);
// Earlier versions of the type checker would incorrectly produce an error
// here stating that there is no type hint on the constant definition.
// We now suppress that error.
