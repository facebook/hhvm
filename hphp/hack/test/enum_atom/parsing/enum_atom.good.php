<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

function foo(): void {
  bar1#A(arg1, arg2);
  SomeClass::bar2#A(arg1, arg2);
  $obj->bar3#A(arg1, arg2);
  bar4<int>#A(arg1, arg2);
}
