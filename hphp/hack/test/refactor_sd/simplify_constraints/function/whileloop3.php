<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {
  $b = f<>;
  $index = 1;
  while (true) {
    if ($index == 9) {
      $index = $b;
      ($index upcast dynamic)();
      break;
    }
    $index++;
  }
}
