<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
    abstract const type T;
    public function get(): this::T;
}

interface BoxWithCtx extends Box {
    abstract const ctx C;
}

type PureBox = BoxWithCtx with { ctx C = [] };
type ZonedBox = BoxWithCtx with { ctx C = [zoned] };
