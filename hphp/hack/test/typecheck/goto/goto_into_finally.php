<?hh // partial

function gotoReferencingLabelInFinally($condition): void {
  goto L0;

  try {
  } finally {
    L0:
  }
}
