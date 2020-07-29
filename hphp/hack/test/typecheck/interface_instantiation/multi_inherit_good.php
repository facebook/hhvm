<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { }

// We allow this. It's caught by a lint rule anyway
class C implements I, I { }
