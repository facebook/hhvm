<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class T {
    public function greet(): string { return "enum_class00... ok!"; }
}

<<__EnumClass>>
class X {
    const C = new T();
}

<<__EntryPoint>>
function enum_class00(): void {
    echo X::C->greet();
}
