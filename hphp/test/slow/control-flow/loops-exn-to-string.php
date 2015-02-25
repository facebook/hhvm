<?hh


// A reduced version of Exception::__toString(). The type of $elem before the
// while loop is Obj<=C, but then it changes to Obj<=C | InitNull inside the
// loop.

class C {
  private ?C $next = null;

  public function __construct(?C $next) {
    $this->next = $next;
  }

  public function iter(): void {
    // Add a preheader to the loop to get interesting behavior.
    if ($this->next === null) return;

    $obj = $this;
    while ($obj !== null) {
      $obj = $obj->next;
    }
  }
}

function main(): void {
  $c = new C(new C(new C(null)));

  $c->iter();
  $c->iter();
  $c->iter();
  $c->iter();
}

main();
