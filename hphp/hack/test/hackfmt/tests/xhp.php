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

function w() {
  return
    <p>
      <my:example:tag />
      <my:exampel:tag:param param={"Hello, World!"} />
    </p>;
}

function x() {
  return $this->:is-active
    ? <span class={cx('myComponent/root', 'myComponent/active')}>
      {$this->getChildren()}
    </span>
    : <a class={cx('myComponent/root')} href={$this->:href}>
      {$this->getChildren()}
    </a>;
}

function y() {
  return
    <div class={cx(Map {
      'linkWrap' => true,
      'hasCount' => $has_count,
      'noCount' => !$has_count,
    })}>
      foo
    </div>;
}

function z() {
  return
    <div>foooooooooooooooooooo{$baaaaaaaaaaaaaaaaaar}<span>baaaaaaaaaaaaaaaaaaz</span><span>qux</span></div>;
}

function aa() {
  $a = <foo></foo>;

  $b = <foo>
       </foo>;

  $c = <foo>
</foo>;

  $d =
    <foo></foo>;

  $e =
    <foo>
    </foo>;

  $f = <foo_______________________________></foo_______________________________>;

  $g = <foo_______________________________>
       </foo_______________________________>;

  $h = <foo_______________________________>
</foo_______________________________>;

  $i =
    <foo_______________________________></foo_______________________________>;

  $j =
    <foo_________________________________></foo_________________________________>;
}

function ab() {
  f(() ==> {
    return <foo___________________>{$bar_______________}</foo___________________>;
  });

  f(
    () ==> {
      return
        <foo___________________>
          {$bar_________________}
        </foo___________________>;
    },
  );
}

function ac() {
  return
    <p>
      <b>{$foo__________________________}</b>
      <i>{$bar__________________________}</i>
    </p>;
}

function ad() {
  return
    <p>
      <b>{$foo__________________________}</b>

      <i>{$bar__________________________}</i>
    </p>;
}

function ae() {
  $x =
    <foo_______________>{$bar__________________________}</foo_______________>;
}
