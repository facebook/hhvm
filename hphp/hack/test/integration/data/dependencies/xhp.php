<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class :xhp implements XHPChild {
  public function __construct(
    darray<string, mixed> $attr,
    varray<mixed> $children,
    string $file,
    int $line
  ) {}

  public function getAttribute(
    string $_attribute
  ): mixed { return null; }
}

class :foo extends :xhp {
  attribute
    int bar @required,
    num baz @lateinit,
    string quux = 'mumble';

  public function render(): :xhp {
    return <xhp>
      bar = {$this->:bar},
      baz = {$this->:baz},
      quux = {$this->:quux}
    </xhp>;
  }
}

function with_xhp(): :xhp {
  return <foo bar={42} />;
}
