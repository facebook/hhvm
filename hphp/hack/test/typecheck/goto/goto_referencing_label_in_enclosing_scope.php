<?hh // partial

function gotoReferencingLabelInEnclosedScope($condition): void {
  L0:

  if ($condition) {
    while (false) {
      goto L0;
    }
  }
}
