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

function i() {
  return <a></a>;
}

function j() {
  return
    <a>
    </a>;
}

function k() {
  return <a><a><a><a><a><a><a><a><a><a><a><a></a></a></a></a></a></a></a></a></a></a></a></a>;
}

function m() {
  return <b><a href="aaaaaaaaaaaaaaaaaaaaaaaaaa" asdf="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"/><a href="a"/></b>;
}

function n() {
  return <a>1 1     1</a>;
}

function o() {
  return <a>
    1 1     1
  </a>;
}

function p() {
  return <aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa href="a" href2="a" href3="a" href4="a">1 1   1</aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa>;
}

function q() {
  return <aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa href="a" href2="a" href3="a" href4="a">
    1 1   1
  </aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa>;
}

function r() {
  return <a><!-- this is a very long comment lorem ipsum ipsum ipsum ipsum ipsum ipsum --></a>;
}

function s() {
  return <a><!-- this is a short comment --></a>;
}

function t() {
  return <a><a href="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa">1</a><a href="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa">1</a></a>;
}

function u($x, $y) {
  return <grid>
    <item>
      {$x}
      {$y}

      {$x}

      {$y}


    </item>
    <item>bar</item>

    <item>bar</item>


    <item>bar</item>
  </grid>;
}

function v() {
  return <a><aaaaaaaaaaa href="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa">1</aaaaaaaaaaa>!</a>;
}

function v() {
  return
    <aaaaaaaaaaaa href="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" name="watttttt"/>;
}
