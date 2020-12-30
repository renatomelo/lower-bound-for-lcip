# Tighter Dual Bounds on the Least Cost Influence Problem
The Least Cost Influence Problem is a combinatorial problem that is usually described in the context of social networks. The objective is to give incentives to a set of individuals in the network, such that some information is spread at minimum cost. 

We provide an efficient algorithm to get lower bounds in a branch-and-bound scheme, and use these in a Branch-and-Cut method.

This project contains the implementation of the combinatorial lower bound for the Least Cost Influence Problem explained in the following cientific paper.

- [Tighter Dual Bounds on the Least Cost Influence Problem](https://proceedings.science/sbpo-2020/papers/tighter-dual-bounds-on-the-least-cost-influence-problem)

## Dependences:
1. SCIP optimization framework version 6.0.1
2. Gurobi optimization solver version 8.1
3. Lemon graph library version 1.3

## Compilation:

`cd lower-bound-for-lcip/`

`make LPS=grb`

## Execution:

`bin/glcip -f <input-file> -a <algorithm> -alpha <value>`

where the input-fle can be any file in `data/`, the algorithm can be `bc`(is the branch-and-cut) or `bc+` (is the branch-and-cut with the lower bound), and the `value` is any number in [0,1] interval.

### Example
`bin/glcip -f data/small/SW-n100-k4-b0.3-d1-10-g0.7-i1.lcip -a bc+ -alpha 1`

To replicate all the experiments done in the paper run the following commands:

For the small world synthetic graphs:
`cd src/; python3  run_sw_instances.py`

For the real-world based graphs:
`cd src/; python3  run_realworld_instances.py`

The algorithm proposed in the paper above is implemented in: `src/dualbound.cpp`

To cite this paper in latex use:

```
@inproceedings{melo2020tighter,
	title={Tighter Dual Bounds on the Least Cost Influence Problem},
	author={de Melo, Renato Silva and Vignatti, Andre Lu{\i}s and Miyazawa, Fl{\'a}vio Keidi and Ota, Matheus Jun},
	booktitle={Proceedings of the 52nd Brazilian Operational Research Symposium},
	year={2020},
	organization={SBPO}
}
```
