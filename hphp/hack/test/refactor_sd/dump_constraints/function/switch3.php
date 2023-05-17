<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $x = g<>;
  $i = 42;
  switch ($i) {
    case 0:
      $x = f<>;
      // FALLTHROUGH
    case 1:
      $x upcast dynamic;
  }
}

function e(): void {
  $x = g<>;
  $i = 42;
  switch ($i) {
    case 0:
      $x = f<>;
      break;
    case 1:
      $x upcast dynamic;
  }
}
