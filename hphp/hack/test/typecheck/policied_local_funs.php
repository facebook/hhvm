<?hh // strict
<<file:__EnableUnstableFeatures('readonly')>>


<<__EntryPoint>>
function test()[zoned_local]: void {
  $f = function() : void {
  };
  $g = () ==> {};
}

function test_leak_safe_local()[leak_safe_local]: void {
  $f = function() : void {
  };
  $g = () ==> {};
}
