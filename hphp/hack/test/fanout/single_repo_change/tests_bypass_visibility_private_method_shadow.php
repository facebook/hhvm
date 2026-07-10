//// base-parent.php
<?hh

class ParentWithPrivateMethod {
  private function target(): void {}
}

//// base-child.php
<?hh

class ChildWithPrivateMethodShadow extends ParentWithPrivateMethod {
  private function target(): void {}
}

//// changed-parent.php
<?hh

class ParentWithPrivateMethod {
  <<__TestsBypassVisibility>>
  private function target(): void {}
}

//// changed-child.php
<?hh

class ChildWithPrivateMethodShadow extends ParentWithPrivateMethod {
  private function target(): void {}
}
