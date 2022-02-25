<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract enum class E: mixed {
  abstract string Y;
}


abstract enum class F: mixed extends E {
}

abstract enum class G: mixed extends E {
  abstract string Z;
}

enum class H: mixed extends F, G {
  string Z = 'yolo';
}

<<__EntryPoint>>
function main(): void {
  echo H::Y;
  echo "\n";
  echo H::Z;
  echo "\n";
}
