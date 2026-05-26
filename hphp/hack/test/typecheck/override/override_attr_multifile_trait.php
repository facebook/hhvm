//// trait_file.php
<?hh

trait OverrideTrait {
  <<__Override>>
  public function needsParent(): void {}
}

//// class_file.php
<?hh

class UsesOverrideTrait {
  use OverrideTrait;
}
