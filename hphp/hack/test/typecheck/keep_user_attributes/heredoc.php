<?hh

final class AAA {

  const DESCRIPTION = <<<'EOT'

some
multiline
description

EOT;

  public function f(): void {
    $x = self::DESCRIPTION;
    hh_show(
      $x,
    ); // Used to show Tany, keep this hh_show to make sure there's no regression
    $this->expect_string(self::DESCRIPTION);
  }

  public function expect_string(string $_): void {}
}
