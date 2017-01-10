<?php
// Tests for PHP-isms that the full-fidelity parser parses in order to
// give good errors should they be used in Hack.
// TODO: Add those errors, and test them.
class foo {
  var $x; // PHP allows "var" as a synonym for "public" on a property.
  function bar() { // PHP allows functions with no modifiers.
    global $a; // Only allowed in PHP, not in Hack.
    global(123); // A statement beginning with global should still parse.
    $f = new foo; // PHP allows argument list to be omitted
    switch($a) { case 1; default; } // PHP allows semis as well as colons
    UNSET($y); // "unset" is case-insensitive
    return $x or $x and $x xor $x <=> $x; // PHP operators.

  }
}
// The closing tag is not supported in Hack but we parse it anyways.
?>
