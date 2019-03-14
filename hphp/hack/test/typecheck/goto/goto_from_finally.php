<?hh // partial

function gotoFromWithinFinally($condition): void {
  try {
  } finally {
    goto L0;
  }

  L0:
}
