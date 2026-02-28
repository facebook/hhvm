<?hh

function foo(int $_):string {
  return "A";
}

function expectFun((function(int):string) $_):void { }
function expectInt(int $_):void { }
function testIt((function():string) $f, (function(int,bool):string) $g):void {
  expectFun($f);
  expectFun($g);
  expectFun(() ==> "A");
  expectFun(($x, bool $b) ==> {
    expectInt($x);
    return "B";
  });
}
