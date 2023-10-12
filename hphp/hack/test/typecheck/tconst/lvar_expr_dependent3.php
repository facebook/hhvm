<?hh // strict

// Test of expression dependent type of local variable for switch/case

abstract class C {
  abstract const type T as num;

  abstract public function get(): this::T;
  abstract public function get_a_C(): C;

  public function test(C $c): void {
    // $c expression id is stable
    hh_show($c->get());
    hh_show($c->get());
    // list assignment generates a new expression id
    list($c) = tuple($c);
    hh_show($c->get());

    switch (0) {
      default:
        // Inside a case the id is still stable
        hh_show($c->get());
        $c = $c->get_a_C();
        // But the above assignment causes a new expression id to be made
        hh_show($c->get());
    }

    // This should be a different expression id since the case may fallthrough
    hh_show($c->get());
    switch (0) {
      case 1:
        // Same expression id as above
        hh_show($c->get());
        // New expression id
        $c = $c->get_a_C();
        hh_show($c->get());
        // FALLTHROUGH
      case 3:
        // New expression id different from $c in 'case 1'
        hh_show($c->get());
        break;
      case 4:
        // Same expression id as outside the switch
        hh_show($c->get());
        $c = $c->get_a_C();
        // New expression id assigned
        hh_show($c->get());
        break;
      default:
        // Same expression id as outside the switch
        hh_show($c->get());
    }
    // New expression id different from everything above
    hh_show($c->get());
  }
}
