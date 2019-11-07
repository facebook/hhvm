/*
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
*/

/// This is a standard nondeterminism monad. This makes it easy to chain
/// together non-deterministic or random computations and then extract the event
/// space of the chain as a list
pub struct M<A>(Vec<A>);

// Equivalent to return in OCaml
pub fn ret<A>(a: A) -> M<A> {
    M(vec![a])
}

impl<A> M<A> {
    /// Equivalent to >>= in OCaml
    pub fn bind<B, F: Fn(A) -> M<B>>(self, f: F) -> M<B> {
        join(self.fmap(f))
    }

    /// Equivalent to >>| in OCaml
    pub fn fmap<B, F: Fn(A) -> B>(self, f: F) -> M<B> {
        M(self.0.into_iter().map(f).collect())
    }

    pub fn add_event(mut self, a: A) -> M<A> {
        self.0.push(a);
        self
    }
}

pub fn join<A>(mm: M<M<A>>) -> M<A> {
    M(mm.0.into_iter().map(|m| m.0).flatten().collect())
}

pub fn lift<'a, A, B, F: Fn(A) -> B>(f: &'a F) -> Box<dyn 'a + Fn(M<A>) -> M<B>> {
    Box::new(move |m: M<A>| m.fmap(f))
}

pub fn ignore(_m: M<()>) {}
