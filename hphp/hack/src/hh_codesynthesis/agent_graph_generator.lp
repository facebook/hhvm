% agent_graph_generator.lp -> Agent Graph generating algorithm in Clingo.
% Can be combined with Clingo Python/Lua interface
% Some placeholders in this file can be run in command line with below section.
% Or "clingo agent_graph_generator.lp -c number_of_agents=1000 -c number_of_roots=1", etc
% =============================================================================
#const number_of_agents = @na().
#const number_of_roots = @nr().
#const number_of_leaves = @nl().
#const number_of_edges = @ne().
#const number_of_infra_agents = @nia().
#const number_of_product_agents = @npa().


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Generating n agents.
% =============================================================================
agents(0..number_of_agents-1).

% An agent X could be an infra agent or a product agent.
1 {infra_agent(X); product_agent(X)} 1 :- agents(X).

% Constraint on number of infra agents
:- #count{ X : infra_agent(X) } < number_of_infra_agents.

% Constraint on number of product agents
:- #count{ X : product_agent(X) } < number_of_product_agents.

% We separated the below part to Python interface to avoid "grounding bottleneck."
% And we are using normal distrubution to create a sequence of "depends_on" among
% n agents. 
% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Minimum critical path requirement.
% =============================================================================
% #const depth = @d().
% path(X, X, 0) :- agents(X).
% path(X, Y, L + 1) :- depends_on(X, Z), path(Z, Y, L), L < depth + 1.
% :- #max{L : path(X, Y, L)} < depth.


% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Degree requirement.
% =============================================================================
% Calculate the in/out degree of each agent.
in_degree_of_agent(X, InDegree) :- agents(X), InDegree = #count{ Y : depends_on(X, Y) }.
out_degree_of_agent(X, OutDegree) :- agents(X), OutDegree = #count{ Y : depends_on(Y, X) }.

% Edge constraint.
% The number of edges in this graph is the sum of all out degree or in degree.
:- Edges = #sum{D : out_degree_of_agent(X, D), agents(X)}, Edges < @ne().

% Constraint on number of roots in the agent graph.
:- #count{ X : in_degree_of_agent(X, 0) } < number_of_roots.

% Constraint on number of leaves in the agent graph.
:- #count{ X : out_degree_of_agent(X, 0) } < number_of_leaves.

% Constraint on each type of the agent.
% If we adjust the function defined in Python, we could specify not only for 
% each agent type, but also for individual agent.
:- out_degree_of_agent(X, D), infra_agent(X), D < @infra_agent_out_degree_low(X).
:- out_degree_of_agent(X, D), infra_agent(X), D >= @infra_agent_out_degree_high(X).

:- in_degree_of_agent(X, D), infra_agent(X), D < @infra_agent_in_degree_low(X).
:- in_degree_of_agent(X, D), infra_agent(X), D >= @infra_agent_in_degree_high(X).

:- out_degree_of_agent(X, D), product_agent(X), D < @product_agent_out_degree_low(X).
:- out_degree_of_agent(X, D), product_agent(X), D >= @product_agent_out_degree_high(X).

:- in_degree_of_agent(X, D), product_agent(X), D < @product_agent_in_degree_low(X).
:- in_degree_of_agent(X, D), product_agent(X), D >= @product_agent_in_degree_high(X).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Edge (Agent relationships) requirement.
% =============================================================================
% If X, Y is number, not string, we can simply write without XN, YN.
{depends_on(Y, X) : X < Y, Y < X + 200} :- agents(X), agents(Y).

{depends_on(Y, X) : X * 3 + 200 < Y} :- product_agent(Y), infra_agent(X).

% {depends_on(Y, X)} :- product_agent(Y), infra_agent(X).
% {depends_on(Y, X)} :- infra_agent(Y), infra_agent(X).
% {depends_on(Y, X)} :- product_agent(Y), product_agent(X).

% Constraint on agent relationship.
% An infra agent shouldn't depend on any product agent.
:- depends_on(Y, X), infra_agent(Y), product_agent(X).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% General Constraints
% =============================================================================
% Print SumOfAllDegree.
% allSD(SD) :- SD = #sum{D : out_degree_of_agent(X, D), agents(X)}.

#show product_agent/1.
#show infra_agent/1.
#show depends_on/2.
