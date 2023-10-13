<?hh

// @partially-generated

// Only Foo::c and Foo::e are formatted--when we have multiple BEGIN tags in a
// row, we consider only the last.

class Foo {
  function a () : int { return 0; } // not formatted
  /* BEGIN MANUAL SECTION */
  function b () : int { return 0; } // not formatted
  /* BEGIN MANUAL SECTION */
  function c () : int { return 0; }
  /* END MANUAL SECTION */
  function d () : int { return 0; } // not formatted
  /* BEGIN MANUAL SECTION */
  function e () : int { return 0; }
  /* END MANUAL SECTION */
  function f () : int { return 0; } // not formatted
}
