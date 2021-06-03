% Given input from "hh_single_type_check --dump-deps --no-builtins"
% We represent them as logic rules in Python interface.
%
% Or we can construct sample input.lp like:
% extends_to(b, a).
% extends_to(b, i1).
% extends_to(b, i2).
% extends_to(i1, i).
% extends_to(i2, i).

% symbols(a; b; i; i1; i2).

% reasoning.lp
%%%%%%%%
% Starter 2/Milestone 1.
%%%%%%%%
% Quickly make sure there is no circular dependencies.
indirect_extends_to(X, Y) :- extends_to(X, Y).
indirect_extends_to(X, Y) :- extends_to(X, Z), indirect_extends_to(Z, Y).

:- extends_to(X, Y), indirect_extends_to(Y, X).

% An symbol could be either class or interface.
1 {class(X); interface(X)} 1 :- symbols(X).

% Later on, we can adjust these parameters to balance number of the classes
% and number of interfaces we expect to synthesis.
#const min_classes = 1.
#const min_interfaces = 1.
:- #count{X: class(X)} < min_classes.
:- #count{X: interface(X)} < min_interfaces.

% If x is an interface, it can only extends_to from other interfaces.
% So, all Y must be an interface.
interface(Y) :- interface(X), extends_to(Y, X).

% If x is a class, it can extends_to from either class or interface.
1 {class(Y); interface(Y)} 1 :- class(X), extends_to(Y, X).
% But, at most 1 parent class is allowed.
:- class(X), #count{Y : class(Y), extends_to(Y, X)} > 1.

% Output, convert to Hack code keywords. Send back to Clingo Python/Lua interface.
% From there, we are using these keywords to produce valid Hack code.
implements(X, Y) :- extends_to(Y, X), class(X), interface(Y).
extends(X, Y) :- extends_to(Y, X), interface(X), interface(Y).
extends(X, Y) :- extends_to(Y, X), class(X), class(Y).
