<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class MyVisitor {
  public function intLiteral(int $i): string {
    return (string)$i;
  }

  public function stringLiteral(string $s): string {
    return "\"$s\"";
  }

  public function plus(mixed $lhs, mixed $rhs): string {
    if ($lhs is string && $rhs is string) {
      return "$lhs + $rhs";
    }
    return "lhs + rhs";
  }

public function call(string $name, vec<mixed> $args): string {
    $call = "$name(";
    foreach ($args as $arg) {
      if ($arg is string) {
        $call = $call . "$arg, ";
      } else {
        $call = $call . "arg, ";
      }
    }
    return $call . ")" ;
  }
}

<<__EntryPoint>>
function test(): void {
  $et = Code`1 + foo("bar")`;

  $visitor = new MyVisitor();

  $res = $et($visitor);

  echo($res);
}
