<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyParent {
  const ctx MyC = [write_props];
  public function callsRequiresLocal()[self::MyC]: void {
    $this->requiresLocal();
  }
  public function requiresLocal()[write_props]: void {}
}
