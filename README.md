# Tighter Dual Bounds on the Least Cost Influence Problem

**Problem**: The Least Cost Influence Problem is a combinatorial problem that is usually described in the context of social networks. The objective is to give incentives to a set of individuals in the network, such that some information is spread at minimum cost. 

We provide an efficient algorithm to get lower bounds in a branch-and-bound scheme, and use these in a Branch-and-Cut method.

This project contains the implementation of the combinatorial lower bound for the Least Cost Influence Problem explained in the following cientific paper.

- [Tighter Dual Bounds on the Least Cost Influence Problem](https://proceedings.science/sbpo-2020/papers/tighter-dual-bounds-on-the-least-cost-influence-problem)

##Dependences:
1. SCIP optimization framework version 6.0.1
2. Gurobi optimization solver version 8.1
3. Lemon graph library version 1.3

##Compilation:

`cd exact-least-cost-influence/`

`make LPS=grb`

##Execution:

`bin/glcip -i <input-file> -a <algorithm> -alpha <value>`

where the input-fle can be any file in `data/`, the algorithm can be `arc`(with is the branch-and-cut) or `arcwb` (branch-and-cut with the lower bound), and the `value` is any number in [0,1] interval.

To replicate all the experiments done in the paper run the following commands:

For the small world synthetic graphs:
`python3  src/run_sw_instances.py`

For the real-world based graphs:
`python3  src/run_realworld_instances.py `
