//// modules.php
<?hh

new module foo {}

//// test.php
<?hh

module foo;
internal enum class E : mixed {
  int A = 42;
}

internal enum class F : mixed extends E {
  int B = 43;
}

internal enum class G : mixed extends E {
  int C = 44;
}

internal enum class H : mixed extends F, G {
  int D = 45;
}

<<__EntryPoint>>
function main(): void {
  echo H::A; echo "\n";
  echo H::B; echo "\n";
  echo H::C; echo "\n";
  echo H::D; echo "\n";
}
