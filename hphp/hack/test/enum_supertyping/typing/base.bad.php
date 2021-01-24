<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum A : int {}

enum B : string {
  use A;
}
