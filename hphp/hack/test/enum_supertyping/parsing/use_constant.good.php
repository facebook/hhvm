<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F : string {}

enum E : string {
  use F;
  USE = "use";
}
