<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
    abstract const type T;
    public function get(): this::T;
}

interface BoxWithCtx extends Box {
    abstract const ctx C super [defaults];
    public function get()[this::C]: this::T;
}

type PureBox = BoxWithCtx with { ctx C = [] };
type ZonedBox = BoxWithCtx with { ctx C = [zoned] };

function get_pure_good(BoxWithCtx with { ctx C = [] } $b)[]: void {
  $b->get();  // OK
}

function get_pure_alias_good(PureBox $b)[]: void {
  $b->get();  // OK
}

function get_pure_bad(BoxWithCtx $b)[]: void {
  $b->get();  // ERROR
}

class IntBox implements Box {
    const type T = int;
    public function get(): int { return 0; }
}
class PureBoxInt extends IntBox implements BoxWithCtx {
    const ctx C = [];
    public function get()[]: int { return 0; }
}

function make_pure_box()[]: BoxWithCtx with { ctx C = [] } {
    return new PureBoxInt();
}

function make_box()[]: BoxWithCtx {
    return make_pure_box();
}

function can_only_call_refined_box(
    BoxWithCtx $maybe_pure_box,
    BoxWithCtx with { ctx C = [] } $pure_box,
)[]: void {
    $pure_box->get();  // OK
    make_pure_box()->get();  // OK

    make_box()->get();  // ERROR
    $maybe_pure_box->get();  // ERROR
}
