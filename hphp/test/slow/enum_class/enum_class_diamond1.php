<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum class E : mixed {
  int A = 42;
}

enum class F : mixed extends E {
  int B = 43;
}

enum class G : mixed extends E {
  int C = 44;
}

enum class H : mixed extends F, G {
  int D = 45;
}

<<__EntryPoint>>
function main(): void {
  echo H::A; echo "\n";
  echo H::B; echo "\n";
  echo H::C; echo "\n";
  echo H::D; echo "\n";
}
