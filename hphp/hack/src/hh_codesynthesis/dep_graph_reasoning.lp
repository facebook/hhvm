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
#const min_classes = @c().
#const min_interfaces = @i().
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

%%%%%%%%
% Milestone 2.
%%%%%%%%
% We represent "Type" and "Method" as logic rules in Python interface.
%
% Or we can construct sample input.lp like:
% extends_to("B", "C").
% method("B", "Foo", "C").
% type("B", "C").

% symbols("B"; "C").

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Type Dependency Edge.
% =============================================================================
% If X needs "Type" check Y, but no "Extend" relationship presents, therefore
% the "Type" edge is coming from a parameter X in a Y method call.
has_method_with_parameter(Y, X) :- type(X, Y), not indirect_extends_to(X, Y).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Method Dependency Edge.
% =============================================================================
% If we see "Method" dependency edge, X must have one method Y.
add_method(X, Y) :- method(X, Y, _).

% Then the "Method" dependency is either coming from
%   1. overriding X::Y in Z. (except Z is an interface, and X is a class.
%           See Constraint 2.1.a)
%   2. invoking X->Y in Z. (except Z is an interface, and it can't invoke.
%           See Constraint 2.2.a)
%
% We say if there is an "Extend" edge and "Method" edge, therefore
% Z overrides X::Y
override(Z, X, Y) :- method(X, Y, Z), extends_to(X, Z).
% We could say if there is no "Extend" edge, but "Method" edge shows up, so
% Z invokes X->Y in any method of Z::__.
% invokes_in_method(Z, X, Y) :- method(X, Y, Z), not extends_to(X, Z).
% However, we give some flexibility to choose from override or invoke a method.
1 {invokes_in_method(Z, X, Y); override(Z, X, Y)} 1 :- method(X, Y, Z).

% As a consequence, if we chose 'override' without an "Extend" edge, we need to
% supplement one "Extend" edge, this ends up the code we synthesized will be a
% Super Set of the given dependency graph, which is totally fine.
extends_to(X, Z) :- override(Z, X, _).
% If Z overrides X::Y, we add a method Y to Z.
add_method(Z, Y) :- override(Z, _, Y).

% Or we chose to invoke X->Y in a method of Z, we must pass X in as a parameter.
has_method_with_parameter(Z, X) :- invokes_in_method(Z, X, Y).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% General Constraints
% =============================================================================
% Constraint 2.1.a: Z overrides X::Y, Z can't be an interface to override a class X.
:- override(Z, X, Y), interface(Z), class(X).

% Constraint 2.2.a: Interface Z can't invoke any method inside.
:- invokes_in_method(Z, _, _), interface(Z).


#show class/1.
#show interface/1.

#show extends/2.
#show implements/2.

#show has_method_with_parameter/2.
#show add_method/2.

#show invokes_in_method/3.
