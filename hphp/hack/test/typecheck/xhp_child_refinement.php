<?hh

class :xhp extends XHPTest {}

function takes_string(string $_): void {}

function main(?string $s): void {
  <xhp>{$s as nonnull}</xhp>;
  takes_string($s);
}
