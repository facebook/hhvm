<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IMyEnt {
  abstract const ctx C super [defaults];
}

abstract class MyBase {
  abstract public function do()[defaults]: void;
}

final class MyReified<reify TEnt as IMyEnt> extends MyBase {
    <<__Override>>
    public function do()[TEnt::C]: void {}
}
