<?hh // strict

namespace NS_names;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

/* test whether keywords are context-sensitive, thus allowing them to be used as names. */

//class abstract {}	// runtime error
//class arraykey {} // name is reserved
//class as {}		// runtime error
//class async {}	// runtime error
//class bool {}	// name is reserved
//class break {}	// runtime error
//class case {}		// runtime error
//class catch {}	// runtime error
//class class {}	// runtime error
//class clone {}	// runtime error   
//class const {}	// runtime error
//class continue {}	// runtime error
//class default {}	// runtime error
//class do {}		// runtime error
//class echo {}		// runtime error
//class else {}		// runtime error
//class elseif {}	// runtime error   
class enum {}   
//class extends {}	// runtime error
//class final {}	// runtime error
//class finally {}	// runtime error
//class float {} // name is reserved
//class for {}		// runtime error
//class foreach {}	// runtime error
//class function {}	// runtime error
//class if {}		// runtime error
//class implements {}	// runtime error
//class instanceof {}	// runtime error
//class insteadof {}	// runtime error
//class int {}	// name is reserved
//class interface {}	// runtime error
//class mixed {} // name is reserved
//class namespace {}	// runtime error
//class new {}		// runtime error
class newtype {}
//class num {}	// name is reserved
//class private {}	// runtime error
//class protected {}	// runtime error
//class public {}	// runtime error
//class require {}	// runtime error
//class require_once {}	// runtime error
//class return {}	// runtime error
//class shape {}	// runtime error; finds a T_SHAPE
//class static {}	// runtime error
//class string {} // name is reserved
//class switch {}	// runtime error
//class throw {}	// runtime error
//class trait {}	// runtime error
//class try {}		// runtime error
class tuple {}
class type {}
//class use {}		// runtime error
//class void {}	// name is reserved
//class while {}	// runtime error
//class yield {}	// runtime error

//class array {} // name is reserved

class null {}
class true {}
class false {}

//function abstract(): void {}	// runtime error
function arraykey(): void {}
//function as(): void {}	// runtime error
function async(): void {}
function bool(): void {}
//function break(): void {}	// runtime error
//function case(): void {}	// runtime error
//function catch(): void {}	// runtime error
//function class(): void {}	// runtime error
//function clone(): void {}	// runtime error   
//function const(): void {}	// runtime error
//function continue(): void {}	// runtime error
//function default(): void {}	// runtime error
//function do(): void {}	// runtime error
//function echo(): void {}	// runtime error
//function else(): void {}	// runtime error
//function elseif(): void {}	// runtime error   
function enum(): void {}   
//function extends(): void {}	// runtime error
//function final(): void {}	// runtime error
//function finally(): void {}	// runtime error
function float(): void {}
//function for(): void {}	// runtime error
//function foreach(): void {}	// runtime error
//function function(): void {}	// runtime error
//function if(): void {}	// runtime error
//function implements(): void {}	// runtime error
//function instanceof(): void {}	// runtime error
//function insteadof(): void {}	// runtime error
function int(): void {}
//function interface(): void {}	// runtime error
function mixed(): void {}
//function namespace(): void {}	// runtime error
//function new(): void {}	// runtime error
function newtype(): void {}
function num(): void {}
//function private(): void {}	// runtime error
//function protected(): void {}	// runtime error
//function public(): void {}	// runtime error
//function require(): void {}	// runtime error
//function require_once(): void {}	// runtime error
//function return(): void {}	// runtime error
//function shape(): void {}	// runtime error; finds a T_SHAPE (not a T_ARRAY, like tuple)
//function static(): void {}	// runtime error
function string(): void {}
//function switch(): void {}	// runtime error
//function throw(): void {}	// runtime error
//function trait(): void {}	// runtime error
//function try(): void {}	// runtime error
//function tuple(): void {}	// runtime error; under the hood, a tuple is an array, hence T_ARRAY
function type(): void {}
//function use(): void {}	// runtime error
function void(): void {}
//function while(): void {}	// runtime error
//function yield(): void {}	// runtime error

//function array(): void {}	// runtime error

function null(): void {}
function true(): void {}
function false(): void {}

class C1 {
  public function abstract(): void {}
  public function arraykey(): void {}
  public function as(): void {}
  public function async(): void {}
  public function bool(): void {}
  public function break(): void {}
  public function case(): void {}
  public function catch(): void {}
  public function class(): void {}
  public function clone(): void {}   
  public function const(): void {}
  public function continue(): void {}
  public function default(): void {}
  public function do(): void {}
  public function echo(): void {}
  public function else(): void {}
  public function elseif(): void {}   
  public function enum(): void {}   
  public function extends(): void {}
  public function final(): void {}
  public function finally(): void {}
  public function float(): void {}
  public function for(): void {}
  public function foreach(): void {}
  public function function(): void {}
  public function if(): void {}
  public function implements(): void {}
  public function instanceof(): void {}
  public function insteadof(): void {}
  public function int(): void {}
  public function interface(): void {}
  public function mixed(): void {}
  public function namespace(): void {}
  public function new(): void {}
  public function newtype(): void {}
  public function num(): void {}
  public function private(): void {}
  public function protected(): void {}
  public function public(): void {}
  public function require(): void {}
  public function require_once(): void {}
  public function return(): void {}
//  public function shape(): void {}	// runtime error; finds a T_SHAPE
  public function static(): void {}
  public function string(): void {}
  public function switch(): void {}
  public function throw(): void {}
  public function trait(): void {}
  public function try(): void {}
  public function tuple(): void {}
  public function type(): void {}
  public function use(): void {}
  public function void(): void {}
  public function while(): void {}
  public function yield(): void {}

  public function array(): void {}

  public function null(): void {}
  public function true(): void {}
  public function false(): void {}

  const int abstract = 100;
  const int arraykey = 100;
  const int as = 100;
  const int async = 100;
  const int bool = 100;
  const int break = 100;
  const int case = 100;
  const int catch = 100;
//  const int class = 100;	// runtime error; finds a T_CLASS
  const int clone = 100;   
  const int const = 100;
  const int continue = 100;
  const int default = 100;
  const int do = 100;
  const int echo = 100;
  const int else = 100;
  const int elseif = 100;   
  const int enum = 100;   
  const int extends = 100;
  const int final = 100;
  const int finally = 100;
  const int float = 100;
  const int for = 100;
  const int foreach = 100;
  const int function = 100;
  const int if = 100;
  const int implements = 100;
  const int instanceof = 100;
  const int insteadof = 100;
  const int int = 100;
  const int interface = 100;
  const int mixed = 100;
  const int namespace = 100;
  const int new = 100;
  const int newtype = 100;
  const int num = 100;
  const int private = 100;
  const int protected = 100;
  const int public = 100;
  const int require = 100;
  const int require_once = 100;
  const int return = 100;
//  const int shape = 100;	// runtime error; finds a T_SHAPE
  const int static = 100;
  const int string = 100;
  const int switch = 100;
  const int throw = 100;
  const int trait = 100;
  const int try = 100;
  const int tuple = 100;
  const int type = 100;
  const int use = 100;
  const int void = 100;
  const int while = 100;
  const int yield = 100;

  const int array = 100;

  const int null = 100;
  const int true = 100;
  const int false = 100;
}

function main(): void {
  new C1();
}

/* HH_FIXME[1002] call to main in strict*/
main();