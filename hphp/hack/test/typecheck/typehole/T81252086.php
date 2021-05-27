<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class :foo {
  attribute string bar;

  public function __construct(
    darray<arraykey,mixed> $_,
    varray<mixed> $_,
    string $_,
        int $_
  ) {}
}

<<__EntryPoint>>
function ok1(): void {
  $xml = <foo bar="hi">Hello</foo>;
  $xml->:bar;
}
