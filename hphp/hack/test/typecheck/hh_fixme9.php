<?hh // partial

function test(): int {
  /* HH_FIXME[4110]: Applied to the line of the next token. */
  /* Even if there is another comment inbetween. */
  return '';
}
