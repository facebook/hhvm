<?hh

abstract class Either<+Tl, +Tr> {
  abstract public function get<Tk>()[]: Tk where Tl as Tk, Tr as Tk;
}

final class Left<+Tl, +Tr> extends Either<Tl, Tr> {
  <<__Override>>
  public function get()[]: Tl {
    throw new Exception('');
  }
}

final class Right<+Tl, +Tr> extends Either<Tl, Tr> {
  <<__Override>>
  public function get()[]: Tr {
    throw new Exception('');
  }
}

final class LogicError {}

function lookup<T>(
  (function(): Either<string, T>) $f,
): Either<LogicError, T> {
  throw new Exception('');
}

function coinflip(int $_): bool {
  return true;
}

abstract final class Foo {
  public static async function do(
  ): Awaitable<?string> {
    if (coinflip(2)) {
      $a = null;
    } else {
      $a = lookup(
        (): Either<string, string> ==> { throw new Exception(''); },
      );
    }
    if ($a is null) {
      $b = null;
    } else {
      $v = $a;
      if ($v is Left<_, _>) {
        $b = null;
      } else {
        if ($v is Right<_, _>) {
          $x = $v->get();
          $b = $x;
        } else {
          invariant_violation('unreachable');
        }
      }
    }
    return $b;
  }
}
