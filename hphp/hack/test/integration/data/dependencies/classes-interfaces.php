<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface BaseInterface {}

interface DerivedInterface extends BaseInterface {}

function with_interface(DerivedInterface $arg): void {}
