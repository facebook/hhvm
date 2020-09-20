<?hh // partial

function test(): int {
  /* HH_FIXME[4110]: when there is nothing after the comment other than
   * white spaces and newline (like in this case), the HH_FIXME is applied
   * to the next line. */
  return '';
}
