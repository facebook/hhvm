<?hh

class D {
  private $x;
  public $y;

  function unsetall() {
    unset($this->x);
    unset($this->y);
  }

  function setprop() {
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() {
    $this->x += 1;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() {
    $this->x++;
    $this->y++;
    var_dump($this);
  }
}
<<__EntryPoint>> function main() {
$d = new D;
// unset all properties
$d->unsetall();
// Prop for visible, accessible property
var_dump($d->y);
// PropU for visible, accessible property
unset($d->y);
// SetProp for visible, accessible properties
$d->setprop();
// SetOpProp
$d->unsetall();
$d->setopprop();
// IncDecProp
$d->unsetall();
$d->incdecprop();
}
