<?php
// Tests for PHP-isms that the full-fidelity parser parses in order to
// give good errors should they be used in Hack.
// TODO: Add those errors, and test them.
class foo {
  var $x; // PHP allows "var" as a synonym for "public" on a property.
  function bar() { // PHP allows functions with no modifiers.
    $f = new foo; // PHP allows argument list to be omitted
    return $x or $x and $x xor $x; // PHP operators with weird precedence.
  }
}
