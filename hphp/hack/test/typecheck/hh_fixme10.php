<?hh // partial

function test(): int {
  /* HH_FIXME[4110]: Applied to the line of the next token, even if there is
   * a blank line inbetween. */

  return '';
}
