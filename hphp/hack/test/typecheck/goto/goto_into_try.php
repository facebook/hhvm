<?hh // partial

function gotoReferencingLabelInEnclosedScope($condition): void {
  goto L0;

  try {
    L0:
  } finally {
  }
}
