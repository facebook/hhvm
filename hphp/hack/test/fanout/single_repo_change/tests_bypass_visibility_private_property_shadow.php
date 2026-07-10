//// base-parent.php
<?hh

class ParentWithPrivateProperty {
  private int $target = 0;
}

//// base-child.php
<?hh

class ChildWithPrivatePropertyShadow extends ParentWithPrivateProperty {
  private int $target = 0;
}

//// changed-parent.php
<?hh

class ParentWithPrivateProperty {
  <<__TestsBypassVisibility>>
  private int $target = 0;
}

//// changed-child.php
<?hh

class ChildWithPrivatePropertyShadow extends ParentWithPrivateProperty {
  private int $target = 0;
}
