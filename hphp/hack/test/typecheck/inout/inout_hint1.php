<?hh // strict

type Tfun = (function(string, inout int): void);

final class C {
  const type T1 = Tfun;
  const type T2 = (function(string, inout int): void);
  const type Tf = (function(this::T1, int): int);
}

function f((function(string, inout int): void) $g, int $x): int {
  $g(__FUNCTION__, inout $x);
  return $x;
}

function f1a(Tfun $g, int $x): int {
  return f($g, $x);
}

function f1b(C::T1 $g, int $x): int {
  return f($g, $x);
}

function f2(C::T2 $g, int $x): int {
  return f($g, $x);
}

function getf(): C::Tf {
  return f<>;
}

function h(string $name, inout int $i): void {
  switch ($name) {
    case 'f':
      $i *= 2;
      break;
    default:
      break;
  }
}

function test(): int {
  $h1 = h<>;
  $h2 = function(string $name, inout int $i): void {
    h($name, inout $i);
  };
  $h3 = (string $name, inout int $i): void ==> {
    h($name, inout $i);
  };

  $f = getf();
  return $f(
    $h1,
    f1a(
      $h2,
      f1b(
        $h3,
        f2(
          ($_, inout $_) ==> {
          },
          21,
        ),
      ),
    ),
  );
}
