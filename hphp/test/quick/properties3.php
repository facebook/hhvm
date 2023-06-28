<?hh

function error_boundary($fn) :mixed{
  try {
    $fn();
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

class D {
  private $x;
  public $y;

  function unsetall() :mixed{
    unset($this->x);
    unset($this->y);
  }

  function setprop() :mixed{
    $this->x = 1;
    $this->y = 2;
    var_dump($this);
  }

  function setopprop() :mixed{
    $this->x ??= 0;
    $this->x += 1;
    $this->y ??= 0;
    $this->y += 2;
    var_dump($this);
  }

  function incdecprop() :mixed{
    error_boundary(() ==> $this->x++);
    error_boundary(() ==> $this->y++);
    var_dump($this);
  }
}
<<__EntryPoint>>
function main() :mixed{
  $d = new D;
  // unset all properties
  $d->unsetall();
  // Prop for visible, accessible property
  try {
    var_dump($d->y);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
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
