<?hh // strict

// @partially-generated

class Foo {
  function a () : int { return 0; }
  /* BEGIN MANUAL SECTION */
  /* END MANUAL SECTION */
  function b () : int { return 0; }

  /* BEGIN MANUAL SECTION */
  function c () : int {
    /* HH_FIXME[4110] */ return vec[ 1 ,
                                     2 ,
                                     3 ];
  }
  /* END MANUAL SECTION */
}
