<?hh

<<__SupportDynamicType>>
function foo(int $_):void { }

function get():~int {
  return 3;
}

function bar():void {
  $x = get();
  foo($x);
  foo(get());
}
