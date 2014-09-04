<?hh

function test(): int {
  /* HH_FIXME[4110]: when there is nothing after the comment other than
   * white spaces and newline, the HH_FIXME is applied
   * to the next line.
   * Doesn't work if there is an extra newline (like in this case). */

  return '';
}
