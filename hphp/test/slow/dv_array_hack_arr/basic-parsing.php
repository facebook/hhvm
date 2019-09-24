<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func1(): varray {}
function func2(): darray {}

function func3(): varray<int> {}
function func4(): darray<int,int> {}

function func5(): array<varray<int>> {}
function func6(): array<darray<int,int>> {}

function func7(): varray<varray<int>> {}
function func8(): varray<darray<int,int>> {}
function func9(): varray<array<int>> {}

function func10(): darray<int,varray<int>> {}
function func11(): darray<int,darray<int,int>> {}
function func12(): darray<int,array<int>> {}

function func13(varray $x) {}
function func14(darray $x) {}

function func15(varray<int> $x) {}
function func16(darray<int,int> $x) {}

function func17(array<varray<int>> $x) {}
function func18(array<darray<int,int>> $x) {}

function func19(varray<varray<int>> $x) {}
function func20(varray<darray<int,int>> $x) {}
function func21(varray<array<int>> $x) {}

function func22(darray<int,varray<int>> $x) {}
function func23(darray<int,darray<int,int>> $x) {}
function func24(darray<int,array<int>> $x) {}

class A {
  public varray $prop1;
  public darray $prop2;

  public varray<int> $prop3;
  public darray<int,int> $prop4;

  public array<varray<int>> $prop5;
  public array<darray<int,int>> $prop6;

  public varray<varray<int>> $prop7;
  public varray<darray<int,int>> $prop8;
  public varray<array<int>> $prop9;

  public darray<int,varray<int>> $prop10;
  public darray<int,darray<int,int>> $prop11;
  public darray<int,array<int>> $prop12;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
