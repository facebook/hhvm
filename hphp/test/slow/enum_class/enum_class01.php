<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class T {
    public function greet(): string { return "enum_class01... ok!"; }
}

class X {
    const C = new T();
}

<<__EntryPoint>>
function enum_class01(): void {
    echo X::C->greet();
}
