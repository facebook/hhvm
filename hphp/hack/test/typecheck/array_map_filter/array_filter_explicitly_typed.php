<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function filter(Container<int> $container) : Container<int> {
  return array_filter<int>($container);
}

function filter_err(Container<int> $container) : Container<int> {
  return array_filter<string>($container);
}
