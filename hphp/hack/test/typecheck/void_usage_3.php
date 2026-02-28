<?hh

function v(): void {}

function test(): void {
  /* HH_IGNORE_ERROR[4273] */
  if (v()) {
  }
}
