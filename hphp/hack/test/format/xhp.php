<?hh

function f() {
  $a = 1;
  return <input data-fooooooooooooooooo={$a} data-baaaaaaaaaaaaaaaaaaaaar={$a} baaaaz={$a}/>;
}

function g() {
  $a = 1;
  return <div data-fooooooooooooooooo={$a} data-baaaaaaaaaaaaaaaaaaaaar={$a} baaaaz={$a}>hi</div>;
}

function h() {
  $a = 1;
  return <input data-foo={$a}/>;
}
