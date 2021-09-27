% agent_graph_generator.lp -> Agent Graph generating algorithm in Clingo.
% Can be combined with Clingo Python/Lua interface
% Some placeholders in this file can be run in command line with below section.
% Or "clingo agent_graph_generator.lp -c number_of_infra_agents=100 
% -c number_of_product_agents=100 ", etc
% =============================================================================
#const number_of_leaves = @nl().
#const number_of_infra_agents = @nia().
#const number_of_product_agents = @npa().

% This is very slow, so we changed to write down the level specifically in the 
% Python interface.
% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Generating n candidate agents.
% =============================================================================
% candidate_agents(0..number_of_agents-1).

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Critical path requirement.
% =============================================================================
% levels(0..number_of_levels-1).
% 1 {agents(X, L): levels(L)} 1 :- candidate_agents(X).
% Constraint on number of agents in each level.
% number_of_agents_at_each_level(Total, L) :- Total = #count{ X : agents(X, L) }, levels(L).
% :- number_of_agents_at_each_level(Total, L), Total < @agents_at_each_level(L).


% An agent X could be an infra agent or a product agent.
1 {infra_agent(X); product_agent(X)} 1 :- agents(X, L).

% Constraint on number of infra agents.
:- #count{ X : infra_agent(X) } < number_of_infra_agents.

% Constraint on number of product agents.
:- #count{ X : product_agent(X) } < number_of_product_agents.

% $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
% Degree requirement.
% =============================================================================
% Calculate the in/out degree of each agent.
in_degree_of_agent(X, InDegree) :- agents(X, _), InDegree = #count{ Y : depends_on(X, Y) }.
out_degree_of_agent(X, OutDegree) :- agents(X, _), OutDegree = #count{ Y : depends_on(Y, X) }.

% Constraint on number of roots in the agent graph.
% :- #count{ X : in_degree_of_agent(X, 0) } < number_of_roots.

% Constraint on number of leaves in the agent graph.
:- #count{ X : out_degree_of_agent(X, 0) } < number_of_leaves.

% Constraint on root agent, must be at level 0.
:- in_degree_of_agent(X, 0), agents(X, Level), Level != 0.

% Constraint on root agent, in degree must be 0.
:- in_degree_of_agent(X, Degree), agents(X, 0), Degree != 0.

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
{depends_on(Y, X) : @agent_connector(Y, X, YLevel, XLevel) != 0} :- 
    agents(X, XLevel), agents(Y, YLevel).

{depends_on(Y, X) : @agent_type_connector(Y, X) != 0} :- product_agent(Y), infra_agent(X).

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
