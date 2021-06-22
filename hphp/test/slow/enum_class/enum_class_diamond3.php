<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {}

class X implements I {
  public function __construct(public string $name)[] {
    echo "Constructing $name\n";
  }
}

enum class E : I {
  X A = new X("A");
}

enum class F : I extends E {
  X B = new X("B");
}

enum class G : I extends E {
  X C = new X("C");
}

enum class H : I extends F, G {
  X D = new X("D");
}

<<__EntryPoint>>
function main(): void {
  echo "From E\n";
  echo E::A->name; echo "\n";
  echo "From F\n";
  echo F::B->name; echo "\n";
  echo "From G\n";
  echo G::C->name; echo "\n";

  echo "From H\n";
  echo H::A->name; echo "\n";
  echo H::B->name; echo "\n";
  echo H::C->name; echo "\n";
  echo H::D->name; echo "\n";
}
