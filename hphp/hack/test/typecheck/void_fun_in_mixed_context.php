<?hh // strict

function expectMixedReturn((function(int):mixed) $f):void { }


function testit():void {
  expectMixedReturn((int $x) ==> { echo "c"; });
}
