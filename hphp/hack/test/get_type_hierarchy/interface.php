<?hh

interface IFoo {}

interface IBar {}

interface IQux extends IFoo, IBar {}
//         ^ type-hierarchy-at-caret
