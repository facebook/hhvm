<?hh // partial

function gotoReferencingLabelInEnclosedScope($condition): void {
  goto L0;

  try {
  } catch (Exception $e) {
    L0:
  }
}
