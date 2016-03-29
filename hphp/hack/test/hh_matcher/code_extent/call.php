<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace my\name;

newtype TestType = (int, int);

const int X = 1;

enum TestEnum : int {
  A = 1;
}

<<__Memoize>>
function test_fun(bool $a, int $b): array<int> {
  if ( $a) {
    echo '} } {';
  } else if ($a) {
    echo '( ( ( )';
  } elseif ($a) {
    echo '{ { { }';
  } else {
    echo "\" { { } }";
  }

  $xhp1 =
    <div>
      test
      <div>nested</div>
    </div>;

  $xhp2 =
    <div>
      test nested braces
      {Vector {
        <div />,
      }}
    </div>;

  $xhp3 =
    <div class={"nested string attribute"}>
      <div class={cx('test')}>
        {'}'}{'{'}
      </div>
    </div>;

  $heredoc2 = <<<TEST
    this only has a newline
TEST
;

  $nowdoc = <<<'TEST'
    this is a nowdoc
TEST;

  switch ($b) {
    case 1:
      echo "1";
      break;
    case 2:
      $heredoc = <<<TAG
some random
multiline
text
TAG;
      echo "1";
      break;
    case 4:
      echo " () (";
      // FALLTHROUGH
    default:
      echo "hi";
      break;
  }

  $i = 0;
  while ($i < 5) {
    $i++;
    if ($i == 3) continue;
    if ($i == 4) break;
    if ($i == 5) return;
    echo $i;
  }

  do {
    $i++;
  } while ($a && (($i < 10) && true));

  $arr = array(1, 2, 3);
  list($el1, $el2, $el3) = $arr;

  foreach ($arr as $elem) {
    echo $elem;
  }

  throw new \Exception('Test Exception');

  return $arr;
}

<<__ConsistentConstruct>>
class Foo {

  <<__Memoize>>
  public function my_method(): void {
    try {
      test_fun(true, 1);
    } catch (\Exception $e) {
      echo $e->getMessage();
    }
    $astring = "astring";
    $anint = (int) $astring;
    $anint = $anint + ( $anint + 1);
    $a = new Bar();
    $c = new \my\name\Foo();
    $d = namespace\A_CONST;
    $e = function() use ($a) {
      $f = (function() use ($a) {
          echo "Hello {}";
          echo 'parenthesis )';
          do { $i++; } while ($a);
        });
    };
  }

  public function my_generator (): Traversable<array<int>> {
    for ($k = 1; $k < 10; $k++) {
      yield array($k);
    }
  }

  public function php_closure (string $who): void  {
    $var = function() use ($who) {
      // FIXME: AST changes broke handling interpolated strings
      //echo "Hello $who {$who + 1}";
      echo "$who{$who + 1}";
    };
  }
}

class Bar {}

async function gen_foo(int $a): Awaitable<?Foo> {
  $bar = await gen_bar($a);
  return new Foo();
}

async function gen_bar(int $a): Awaitable<?Bar> {
  return new Bar();
}

const int A_CONST = 500;
