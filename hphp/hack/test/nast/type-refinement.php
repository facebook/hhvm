<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class Box {
  abstract const type Tb as mixed;
  public function __construct(public this::Tb $val) { }
}

type IBox = Box with { type Tb = int };
type SubIBox = Box with { type Tb as int };
type SubISBox = Box with { type Tb as int as string };
type SupIBox = Box with { type Tb super int };
type SupISBox = Box with { type Tb super int super string };

type SubSupBox = Box with { type Tb as int super int };
