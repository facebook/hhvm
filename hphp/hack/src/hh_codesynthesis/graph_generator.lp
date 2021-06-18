% generate_gen.lp -> Graph generating algorithm in Clingo.
% Can be combined with Clingo Python/Lua interface, dep_graph_reasoning.lp
% Some placeholders in this file can be run in command line with below section.
% Or "clingo generate_gen.lp -c n=10 -c avg_width=1", etc
% =============================================================================
#const n = @n().
#const avg_width = @w(). % 1 out of 10th scale, this means average = 0.1


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Genrating n symbols.
% =============================================================================
% The actual symbol created in Python interface is internal_symbols/2,
% The first arity is a string, the second arity is a number.
% like, internal_symbols("S0", 0).
% We are converting it to symbols/1. here, simply ignore the second arity with "_".
symbols(X) :- internal_symbols(X, _).
% Uncomment this for an easy prototyping without Python interface.
% symbols(1..n).
% internal_symbols(_, X) :- symbols(X).


% We separated the below part to Python interface to avoid "grounding bottleneck."
% And we are using normal distrubution to create a sequence of "extends_to" among
% n nodes. For n = 100, we can use
% extends_to(1, 11).
% extends_to(11, 21).
% extends_to(21, 31).
% extends_to(31, 41).
% extends_to(41, 51).
% extends_to(51, 61).
% extends_to(61, 71).
% extends_to(71, 81).
% extends_to(81, 91).
% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Minimum depth requirement.
% =============================================================================
% #const depth = @d().
% path(X, X, 0) :- symbols(X).
% path(X, Y, L + 1) :- extends_to(X, Z), path(Z, Y, L), L < depth + 1.
% :- #max{L : path(X, Y, L)} < depth.


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Average width requirement.
% =============================================================================
% Calculate the degree of each symbol.
degree(X, D) :- symbols(X), D = #count{ Y : extends_to(Y, X) }.
% One class or interface at most can have 10 extends relationship.
% We can adjust this one later, base on the profiling from actual software.
% We believed if a code has more than 10 extends/implements is a bad design.
% Therefore, we added this constraint to prevent AI synthesis bad code.
:- degree(X, D), D > 10.
% Average constraint.
:- SumOfAllDegree = #sum{D : degree(X, D), symbols(X)},
    SumOfAllDegree * 10 < avg_width * n.
% Notice above, we can't write SumOfAllDegree/n < avg_width, since Clingo only supports
% integers. (a.k.a countable set, Real number is an uncountable set.)
% Therefore the parameter average_width actually is 1 out of 10th scale.


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Spanning requirement.
% =============================================================================
{extends_to(Y, X) : XN + @lb() < YN, YN < XN + @hb()} :-
    internal_symbols(X, XN), internal_symbols(Y, YN).
% If X, Y is number, not string, we can simply write without XN, YN.
% {extends_to(X, Y) : X + 10 < Y, Y < X + 100} :- symbols(X), symbols(Y).


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% General Constraints
% =============================================================================
% Print SumOfAllDegree.
% allSD(SD) :- SD = #sum{D : degree(X, D), symbols(X)}.
