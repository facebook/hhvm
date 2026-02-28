<?hh

function take_lambda<T>(T $x, (function(T): void) $f): void {
  ($f)($x);
}

function f_int(?int $nullable_int): void {
  take_lambda($nullable_int, $i ==> {
    $_ = $i > 100; // error, $i might be null
  });
}

function f_string(?string $nullable_str): void {
  take_lambda($nullable_str, $i ==> {
    $_ = "" > $i; // error, $i might be null
  });
}

<<__EntryPoint>>
function main(): void {
  f_int(null);
}
