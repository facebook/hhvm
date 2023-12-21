<?hh

if (
  $predicate // pred 1
  || $other_predicate // pred 2
) {
  // ...
}

if (
  $first_predicate // 1
  || $second_predicate // 2
  || $third_predicate // 3
  || $fourth_predicate // 4
) {
  // ...
}

if (
  $predicate || // pred 1
  $other_predicate // pred 2
) {
  // ...
}

if (
  $first_predicate || // 1
  $second_predicate || // 2
  $third_predicate || // 3
  $fourth_predicate // 4
) {
  // ...
}

if (
  $first_predicate || // UNSAFE
  $second_predicate // FALLTHROUGH
) {
  // ...
}

$sum = $num1 // num1
  + $num2; // num2

$sum = $number_with_long_identifier1 // num1
  + $number_with_long_identifier2 // num2
  + $number_with_long_identifier3; // num3

$sum = $num1 + // num1
  $num2; // num2

$sum = $number_with_long_identifier1 + // num1
  $number_with_long_identifier2 + // num2
  $number_with_long_identifier3; // num3

function_call(
  $num1 // num1
  + $num2, // num2
);

function_call(
  $number_with_long_identifier1 // num1
  + $number_with_long_identifier2 // num2
  + $number_with_long_identifier3, // num3
);

function_call(
  $num1 + // num1
  $num2, // num2
);

function_call(
  $number_with_long_identifier1 + // num1
  $number_with_long_identifier2 + // num2
  $number_with_long_identifier3, // num3
);

function_call(
  $num // num
);

function_call(
  $num1, // num1
  $num2 // num2
);

function_call(
  $number_with_incredibly_unbelievably_surprisingly_long_identifier // long!
);

function_call(
  $number_with_long_identifier1, // num1
  $number_with_long_identifier2, // num2
  $number_with_long_identifier3 // num3
);

function_call
  ( $num // num
  );

function_call
  ( $num1 // num1
  , $num2 // num2
  );

function_call
  ( $number_with_incredibly_unbelievably_surprisingly_long_identifier // long!
  );

function_call
  ( $number_with_long_identifier1 // num1
  , $number_with_long_identifier2 // num2
  , $number_with_long_identifier3 // num3
  );

$arr[
  "idx" // some idx
] = $val;

function f(
  int $a, // arg1
) {
  // ...
}

function f(
  int $a // arg1
) {
  // ...
}

function f(
  int $a, // arg1
  int $b, // arg2
) {
  // ...
}

function f(
  int $a, // arg1
  int $b // arg2
) {
  // ...
}

function f
  ( int $a // arg1
  )
{
  // ...
}

function f
  ( int $a // arg1
  , int $b // arg2
  )
{
  // ...
}

function getArray(
): array<
  string, // strings
  string // strings
> {
  return dict["foo" => "bar"];
}

function getArray(
): array< string // strings
        , string // strings
        > {
  return dict["foo" => "bar"];
}

function getDict(
): dict<
  string, // strings
  string // strings
> {
  return dict["foo" => "bar"];
}

function getDict(
): dict< string // strings
        , string // strings
        > {
  return dict["foo" => "bar"];
}

function swap<
  T, // T
  U // U
>(
  Vector<T> $a, // T
  Vector<U> $b // U
): Pair<
  Vector<U>, // U
  Vector<T> // T
> {
  return Pair { $b, $a };
}

function swap
  < T // T
  , U // U
  >( Vector<T> $a // T
   , Vector<U> $b // U
   ): Pair< Vector<U> // U
          , Vector<T> // T
          > {
  return Pair { $b, $a };
}
