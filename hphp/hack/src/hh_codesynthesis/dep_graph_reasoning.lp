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

% A symbol is unique, can't be a class/interface name and a function name
% at the same time.
:- symbols(X), funcs(X).

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
% Type Dependency Edge <Type, Type>.
% =============================================================================
% If TypeX needs "Type" check TypeY, but no "Extend" relationship presents, so
% the "Type" edge is coming from a parameter TypeX in a TypeY method call.
has_method_with_parameter(TypeY, TypeX) :-
    type(TypeX, TypeY),
    not indirect_extends_to(TypeX, TypeY),
    symbols(TypeY).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Method Dependency Edge <Method, Type>.
% =============================================================================
% If we see "Method" dependency edge, TypeX must have one method with MethodName.
add_method(TypeX, MethodName) :- method(TypeX, MethodName, _).

% Then the "Method" dependency is either coming from
%   1. overriding TypeX::MethodName in TypeZ. (except TypeZ is an interface,
%           and TypeX is a class. See Constraint 2.1.a)
%   2. invoking TypeX->MethodName in TypeZ. (except TypeZ is an interface,
%           and it can't invoke. See Constraint 2.2.a)
%
% #############################################################################
% Using some greedy approach in the model to reduce the usage of "choice rule".
% Rule 2.1.1: We say if there is an "Extend" edge and "Method" edge, therefore
% TypeZ overrides TypeX::MethodName
% override(TypeZ, TypeX, MethodName) :-
%               method(TypeX, MethodName, TypeZ), extends_to(TypeX, TypeZ).
% Rule 2.1.2: We could say if there is no "Extend" edge, but "Method" edge
% shows up, so TypeZ invokes TypeX->MethodName in any method of TypeZ::__.
% invokes_in_method(TypeZ, TypeX, MethodName) :-
%               method(TypeX, MethodName, TypeZ), not extends_to(TypeX, TypeZ).
% #############################################################################
% Rule 2.1.3: However, we give complete flexibility to choose from override or
% invoke a method.
1 {invokes_in_method(TypeZ, TypeX, MethodName);
    override(TypeZ, TypeX, MethodName)} 1 :-
        method(TypeX, MethodName, TypeZ), symbols(TypeZ).
%
% Currently, it's "non-deterministic" for "Method" edges no matter how "Extends"
% layouts are. If there is a performance issue, feel free to mix up Rule 2.1.1-3
% The performance impact is depending on the size of intersection of "Method"
% and "Extends". If we have some overlapping between "Method" <-> "Extends" we
% deterministically decide "override" using Rule 2.1.1. Therefore, we saved some
% time. We can also enable Rule 2.1.2, so that we deterministically decide
% "invoke". And saved more time. Remember, any combination is valid under
% Declarative Programming.


% As a consequence, if we chose 'override' without an "Extend" edge, we need to
% supplement one "Extend" edge, this ends up the code we synthesized will be a
% Super Set of the given dependency graph, which is totally fine.
extends_to(TypeX, TypeZ) :- override(TypeZ, TypeX, _).
% If TypeZ overrides TypeX::MethodName, we add a method Methodname to TypeZ.
add_method(TypeZ, MethodName) :- override(TypeZ, _, MethodName).

% Or we chose to invoke TypeX->MethodName in a method of TypeZ, we must pass
% TypeX in as a parameter.
has_method_with_parameter(TypeZ, TypeX) :- invokes_in_method(TypeZ, TypeX, _).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% General Constraints
% =============================================================================
% Constraint 2.1.a: TypeZ overrides TypeX::_, TypeZ can't be an
% interface to override a class TypeX.
:- override(TypeZ, TypeX, _), interface(TypeZ), class(TypeX).

% Constraint 2.2.a: Interface TypeZ can't invoke any method inside.
:- invokes_in_method(TypeZ, _, _), interface(TypeZ).

%%%%%%%%
% Milestone 3.
%%%%%%%%
% We represent "FUN" and "SMethod" as logic rules in Python interface.
%
% Or we can construct sample input.lp like:
% extends_to("A", "B").
% method("A", "foo", "B").
% type("A", "B").
% type("B", "C").
% type("C", "B").
%
% invoked_by("fn_bar", "B").
% invoked_by("fn_foo", "B").
% method("C", "call", "fn_foo").
% method("B", "foo", "fn_bar").
% static_method("B", "Bar", "fn_bar").
% static_method("B", "Bar", "C").
%
% type("B", "fn_bar").
% type("C", "fn_foo").
%
% funcs("fn_foo"; "fn_bar").
% symbols("A"; "B"; "C").

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% SMethod Dependency Edge <SMethod, Type>, <SMethod, Fun>.
% =============================================================================
% If TypeX has a static method, TypeX must be a class.
class(TypeX) :- static_method(TypeX, _, _).

% Let's add a static method to the class.
add_static_method(ClassName, MethodName) :- static_method(ClassName, MethodName, _).

% If TypeX is a class or function, and we see a static method edge, then TypeX
% invokes ClassName::MethodName.
invokes_static_method(TypeX, ClassName, MethodName) :-
                static_method(ClassName, MethodName, TypeX).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% SMethod Constraints
% =============================================================================
% If TypeX is an interface, it can't invoke a static method.
:- static_method(_, _, TypeX), interface(TypeX).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Fun Dependency Edge <Fun, Type>, <Fun, Fun>.
% =============================================================================
% TypeX could be either a function or a class to invoke FnName.
invokes_function(TypeX, FnName) :- invoked_by(FnName, TypeX).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Fun Constraints
% =============================================================================
% If TypeX is an interface, it can't invoke a function.
:- invoked_by(_, TypeX), interface(TypeX).


#show class/1.
#show interface/1.
#show funcs/1.

#show extends/2.
#show implements/2.

#show has_method_with_parameter/2.
#show invokes_function/2.

#show add_method/2.
#show add_static_method/2.

#show invokes_in_method/3.
#show invokes_static_method/3.
