<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class MyParent {
  const ctx MyC = [local];
  public function callsRequiresLocal()[self::MyC]: void {
    $this->requiresLocal();
  }
  public function requiresLocal()[local]: void {}
}
