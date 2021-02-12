<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum B: string as string {
   use A;
}
enum A: string as string {
   FOO = 'foo';
}
<<__EntryPoint>>
function main() : void {
   print B::FOO;
}
