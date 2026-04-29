<?hh
<<file:
  __EnableUnstableFeatures(
    'polymorphic_lambda',
    'polymorphic_function_hints',
  )>>

// Church-encoded free monad, abstracted over the effect type via the Op<TResp>
// interface.
//
// GADT SIMULATION
//
// Hack simulates GADTs with interfaces and phantom type parameters. The
// Op<TResp> interface is the abstraction point — any effect set implements it
// via a sealed abstract class, and each operation fixes TResp:
//
//   interface Op<TResp> {}
//   <<__Sealed(PrintOp::class, ReadOp::class)>>
//   abstract class ConsoleOp<TResp> implements Op<TResp> {}
//   class PrintOp extends ConsoleOp<null>   { ... }
//   class ReadOp  extends ConsoleOp<string> { ... }
//
// The sealed class gives each effect set a closed set of operations for
// exhaustive matching, while Op<TResp> lets Free<T> work with any effect
// set. FreeOps (pure, perform, bind, map) are effect-agnostic; only the
// interpreter is effect-specific.
// This gives type-safe per-operation response types: perform(new PrintOp(...))
// returns Free<null>, perform(new ReadOp()) returns Free<string>.
//
// GADT ELIMINATION
//
// Hack refines the type parameter TResp when pattern-matching with `is`.
// After `$op is PrintOp`, the checker knows TResp = null, so calling
// $k(null) on a continuation (function(TResp):R) type-checks. This means
// operations are pure data. Could also be implemented with dynamic dispatch
//
// FREE MONAD ENCODING
//
// The free monad uses a handler universally quantified over the response type:
//
//   Free<T> ≅ ∀R. (T → R) → (∀U. Op<U> → (U → R) → R) → R
//
// The ∀U makes the handler work for any operation regardless of its response
// type. Each call to the handler instantiates U to the specific operation's
// TResp. The continuation (U → R) sequences effects correctly in a strict
// language without thunking — the continuation is a function, so it's only
// evaluated when the handler calls it.
//
// IMPREDICATIVE POLYMORPHISM
//
// Hack's polymorphism is impredicative: type variables can be instantiated
// with polymorphic types, not just ground types.
//
// run_with_backend demonstrates rank-3. It takes a "suite" callback whose
// own argument is a polymorphic interpreter (function<T>(Free<T>): T).
// That puts ∀T two arrows deep in run_with_backend's signature:
//
//   run_with_backend : ... → ((∀T. Free<T> → T) → void) → void
//
// The suite receives a polymorphic interpreter and applies it at multiple
// types (T=null for print_, T=string for read_). The backend constructs
// the interpreter — dependency injection at rank-3.

interface Op<TResp> {}

<<__Sealed(PrintOp::class, ReadOp::class)>>
abstract class ConsoleOp<TResp> implements Op<TResp> {}

class PrintOp extends ConsoleOp<null> {
  public function __construct(public string $msg) {}
}

class ReadOp extends ConsoleOp<string> {}

newtype Free<T> = (function<R>(
  (function(T): R),
  (function<U>(Op<U>, (function(U): R)): R),
): R);

class FreeOps {

  public static function pure<T>(T $x): Free<T> {
    return (
      function<R>(
        (function(T): R) $on_pure,
        (function<U>(Op<U>, (function(U): R)): R) $_on_op,
      ): R ==> $on_pure($x)
    );
  }

  public static function perform<TResp>(Op<TResp> $op): Free<TResp> {
    return (
      function<R>(
        (function(TResp): R) $on_pure,
        (function<U>(Op<U>, (function(U): R)): R) $on_op,
      ): R ==> $on_op($op, $resp ==> $on_pure($resp))
    );
  }

  public static function map<TIn, TOut>(
    (function(TIn): TOut) $f,
    Free<TIn> $m,
  ): Free<TOut> {
    return (
      function<R>(
        (function(TOut): R) $on_pure,
        (function<U>(Op<U>, (function(U): R)): R) $on_op,
      ): R ==> $m($a ==> $on_pure($f($a)), $on_op)
    );
  }

  public static function bind<TIn, TOut>(
    (function(TIn): Free<TOut>) $f,
    Free<TIn> $m,
  ): Free<TOut> {
    return (
      function<R>(
        (function(TOut): R) $on_pure,
        (function<U>(Op<U>, (function(U): R)): R) $on_op,
      ): R ==> $m($a ==> ($f($a))($on_pure, $on_op), $on_op)
    );
  }

  public static function then_<TFirst, TOut>(
    Free<TOut> $next,
    Free<TFirst> $first,
  ): Free<TOut> {
    return self::bind($_ ==> $next, $first);
  }

  public static function interpret<T>(
    (function(string): void) $do_print,
    (function(): string) $do_read,
    Free<T> $program,
  ): T {
    return $program(
      $a ==> $a,
      (
        function<TResp>(Op<TResp> $op, (function(TResp): T) $k): T ==> {
          if ($op is PrintOp) {
            $do_print($op->msg);
            return $k(null);
          }
          invariant($op is ReadOp, 'unexpected op');
          return $k($do_read());
        }
      ),
    );
  }

}

function print_(string $msg): Free<null> {
  return FreeOps::perform(new PrintOp($msg));
}

function read_(): Free<string> {
  return FreeOps::perform(new ReadOp());
}

function example_program(): Free<string> {
  $ask = print_("What is your name?");
  $ask_then_read = FreeOps::then_(read_(), $ask);
  $greet = FreeOps::bind($name ==> print_("Hello, ".$name."!"), $ask_then_read);
  return FreeOps::then_(FreeOps::pure("done"), $greet);
}

// Rank-3: run_with_backend takes a suite, which is rank-2 (it takes a
// polymorphic interpreter as argument). So run_with_backend's argument
// has ∀ nested two arrows deep:
//
//   run_with_backend : ... → ((∀T. Free<T> → T) → void) → void
//                                  ↑ rank-1       ↑ rank-2    ↑ rank-3
function run_with_backend(
  (function(string): void) $do_print,
  (function(): string) $do_read,
  (function((function<T>(Free<T>): T)): void) $suite,
): void {
  $suite(
    function<T>(Free<T> $prog): T ==>
      FreeOps::interpret($do_print, $do_read, $prog),
  );
}

<<__EntryPoint>>
function main(): void {
  $pure = FreeOps::interpret($_msg ==> {}, () ==> "", FreeOps::pure("hello"));
  echo "pure: ".$pure."\n";

  $result = FreeOps::interpret(
    $msg ==> { echo "  > ".$msg."\n"; },
    () ==> "Alice",
    example_program(),
  );
  echo "result: ".$result."\n";

  echo "--- run_with_backend ---\n";
  run_with_backend(
    $msg ==> { echo "  > ".$msg."\n"; },
    () ==> "Bob",
    $interpret ==> {
      $interpret(print_("hello"));
      $interpret(print_("world"));
      $name = $interpret(read_());
      $result2 = $interpret(example_program());
      echo $name." ".$result2."\n";
    },
  );
}
