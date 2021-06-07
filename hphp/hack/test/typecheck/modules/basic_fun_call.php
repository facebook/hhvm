<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
function a(): void {}

<<__Module("A")>>
function a2(): void { a(); /* ok */ }

<<__Module("B")>>
function b(): void { a(); /* error */ }

<<__Module("B")>>
function b2(): void { a2(); /* ok */ }

function main(): void { a(); /* error */ }
