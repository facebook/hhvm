<?hh // partial

function gotoReferencingLabelInEnclosedScope($condition): void {
  goto L0;

  if ($condition) {
    while (false) {
      L0:
    }
  }
}
