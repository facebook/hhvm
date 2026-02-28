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
      $x upcast dynamic;
      $x = f<>;
      break;
    case 1:
      f<> upcast dynamic;
      break;
    default:
      g<> upcast dynamic;
  }
  $x upcast dynamic;
}

function e(): void {
  $x = f<>;
  $i = 42;
  switch ($i) {
    case 0:
      $x = g<>;
      break;
    case 1:
      $x = g<>;
      break;
    case 2:
      $x = g<>;
      break;
    default:
      $x = g<>;
  }
  $x upcast dynamic;
}

function d(): void {
  $x = g<>;
  $i = 42;
  switch ($i) {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      $x = f<>;
      break;
    case 5:
      break;
    default:
  }
  $x upcast dynamic;
}
