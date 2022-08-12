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

type PureBox = BoxWithCtx with { ctx C super [] };
type ImpureBox = BoxWithCtx with { ctx C as [] };
type BoxThatWritesStuff = BoxWithCtx with { ctx C as [globals, write_props] };

type BoxThatOnlyAccessesGlobals = BoxWithCtx with {
    ctx C as [read_globals] super [globals]
};
