<?hh // partial

/**
 * Be aware that:
 *
 *   array<int, string> <: array<string>
 *
 * But:
 *
 *   array<string> !<: array<int, string>
 *
 * This test is the analog of the above, but with darray.
 */

function test(): array<string> {
  return darray[0 => "tingley"];
}
