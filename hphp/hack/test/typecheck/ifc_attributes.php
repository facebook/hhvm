<?hh
<<file:__EnableUnstableFeatures('ifc')>>
// Methods
class C {
  <<__Policied("Test")>>
  public function test(): void {}

  <<__Policied("Public")>>
  public function pub(): void {}

  <<__Policied>>
  public function implicit(): void {}

  <<__InferFlows>>
  public function inferflows(): void {}

  public function defaultflow(): void {}

  <<__Policied("A", "B")>>
  public function wrong(): void {}

  <<__InferFlows("A")>>
  public function wrong2(): void {}
}

// Toplevel functions
function test(): void {}

<<__Policied("Public")>>
function pub(): void {}

<<__Policied>>
function implicit(): void {}

<<__InferFlows>>
function inferflows(): void {}

function defaultflow(): void {}

<<__Policied("A", "B")>>
function wrong(): void {}

<<__InferFlows("A")>>
function wrong2(): void {}
