<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

<<__SupportDynamicType>>
class Box<T> {}

<<__SupportDynamicType>>
class C{
  public static ~?supportdyn<(function(int):string)> $prop = null;
  public static ~?Box<supportdyn<(function(int):string)>> $prop2 = null;
}

function test(~?(function(int):string) $f) : void {}

function one(~HH\FunctionRef<supportdyn<(function(int): string)>> $f): void {
}

function onebox(~Box<supportdyn<(function(int): string)>> $f): void {
}

function two(HH\FunctionRef<supportdyn<(function(~int):~string)>> $v) : void {
  one($v);
}

function twobox(Box<supportdyn<(function(~int):~string)>> $v) : void {
  onebox($v);
}
