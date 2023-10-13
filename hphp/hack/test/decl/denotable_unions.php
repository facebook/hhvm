<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
interface J {}

function f((I& J) $_x): void {}
function g((I| J) $_x): void {}
