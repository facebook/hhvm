<?hh

class B {
  public int $i = 0;
  public readonly function this_readonly(): void { }
  public function this_not_readonly():void { $this->i = 3; }

}

function expectRR((readonly function(readonly B):void) $_):void { }
function expectRN((readonly function(B):void) $_):void { }

function test(): void {
  // Control: we expect these lambdas to be readonly (no capture)
  $f = readonly (readonly B $x) ==> $x->this_readonly();
  expectRR($f);
  $g = readonly (B $x) ==> $x->this_not_readonly();
  expectRN($g);

  // This should have the same type as $f
  $h = meth_caller(B::class, 'this_readonly');
  expectRR($h);
  // This should have the same type as $g
  $i = meth_caller(B::class, 'this_not_readonly');
  expectRN($i);
}
