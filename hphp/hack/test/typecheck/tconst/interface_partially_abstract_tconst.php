<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// Interface cannot contain a type constant that is partially abstract
interface I {
  const type T as int = int;
}
