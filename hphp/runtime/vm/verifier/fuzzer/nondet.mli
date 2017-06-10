(*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*)

(* This is a standard nondeterminism monad. This makes it easy to chain
together non-deterministic or random computations and then extract the event
space of the chain as a list *)
type 'a m

val bind : 'a m -> ('a -> 'b m) -> 'b m

val return : 'a -> 'a m

val fmap :  'a m -> ('a -> 'b) -> 'b m

val lift : ('a -> 'b) -> ('a m -> 'b m)

val join  : 'a m m -> 'a m

val (>>=) : 'a m -> ('a -> 'b m) -> 'b m

val (>>|) :  'a m -> ('a -> 'b) -> 'b m

val add_event : 'a m -> 'a -> 'a m

val ignore : unit m -> unit
