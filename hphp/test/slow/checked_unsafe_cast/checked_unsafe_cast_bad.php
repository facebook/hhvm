<?hh

function cast_bad(mixed $m):void {
  $mc = HH\FIXME\UNSAFE_CAST<mixed, DoesNotExist>($m);
}

<<__EntryPoint>>
function main():void {
  cast_bad(3);
}
