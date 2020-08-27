<?hh
<<file:__EnableUnstableFeatures(
    'pocket_universes',
)>>

class PU2 {
  enum E {
    case type T;
    case (function(): T) generator;
    case (function(T): void) logger;
    :@string_message(
      type T = string,
      generator = function(): string {
        return "hello\n";
      },
      logger = function(string $x): void {
        echo "string_message_logger: ";
        echo $x;
      }
    );
    :@int_message(
      type T = int,
      generator = function(): int {
        return 42;
      },
      logger = function(int $x): void {
        echo "int_message_logger: ";
        echo $x;
      }
    );
  }
}

<<__EntryPoint>>
function main(): void {
  foreach (PU2:@E::Members() as $member1) {
    foreach (PU2:@E::Members() as $member2) {
      $gen = PU2:@E::generator($member2);
      $log = PU2:@E::logger($member1);
      $x = $gen();
      $log($x);
    }
  }
}
