<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F : int {
}

enum E : int {
  USE = "use";
  use F;
}
