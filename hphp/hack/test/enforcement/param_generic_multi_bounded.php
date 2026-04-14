<?hh

interface I {}
interface J {}

function foo<T as I as J as arraykey>(T $x): void {}
//                                    ^ enforcement-at-caret
