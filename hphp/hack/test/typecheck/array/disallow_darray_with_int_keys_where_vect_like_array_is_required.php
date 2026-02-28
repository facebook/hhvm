<?hh

/**
 * Be aware that:
 *
 *   darray<int, string> <: varray<string>
 *
 * But:
 *
 *   varray<string> !<: darray<int, string>
 *
 * This test is the analog of the above, but with darray.
 */

function test(): varray<string> {
  return dict[0 => "tingley"];
}
