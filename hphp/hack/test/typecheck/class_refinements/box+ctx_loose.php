<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
    abstract const type T;
    public function get(): this::T;
}

interface BoxWithCtx extends Box {
    abstract const ctx C;
}

type BoxThatWritesGlobals = BoxWithCtx with { ctx C as [globals] };
type WritePropsOrPurerBox = BoxWithCtx with { ctx C super [write_props] };
