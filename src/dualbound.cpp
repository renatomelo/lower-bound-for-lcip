#include "dualbound.h"
#include <lemon/hao_orlin.h>

DualBound::DualBound(
    SCIP *scip,
    GLCIPInstance &p_instance,
    DNodeSCIPVarMap &p_x,
    ArcSCIPVarMap &p_z) : ObjRelax(scip,
                                   "heuristic-dual-bound",
                                   "Heuristic dual bound for GLCIP",
                                   -1.0,   //priority of the relaxator (negative: after LP, non-negative: before LP)
                                   1,      //frequency for calling relaxator
                                   FALSE), //Does the relaxator contain all cuts in the LP?
                          instance(p_instance),
                          x(p_x),
                          z(p_z)
{
}

SCIP_DECL_RELAXFREE(DualBound::scip_free)
{
    return SCIP_OKAY;
}

SCIP_DECL_RELAXINIT(DualBound::scip_init)
{
    return SCIP_OKAY;
}

SCIP_DECL_RELAXEXIT(DualBound::scip_exit)
{
    return SCIP_OKAY;
}

SCIP_DECL_RELAXINITSOL(DualBound::scip_initsol)
{
    return SCIP_OKAY;
}

SCIP_DECL_RELAXEXITSOL(DualBound::scip_exitsol)
{
    return SCIP_OKAY;
}

void getCondensedGraph(
    GLCIPInstance &instance,
    Digraph &condensed,
    Digraph &graph,
    DNodeDNodeMap &nodeRef,
    Digraph::NodeMap<int> &components,
    vector<vector<DNode>> &listOfComponents)
{
    //Digraph condensed;
    int nComponents = countStronglyConnectedComponents(graph);
    for (int i = 0; i < nComponents; i++)
    {
        condensed.addNode();
    }

    //compute the components of 'graph'
    stronglyConnectedComponents(graph, components);

    //save each component in a vector of vertices
    for (DNodeIt v(graph); v != INVALID; ++v)
        listOfComponents[components[v]].push_back(nodeRef[v]);
}

vector<double> getCondensedThresholds(
    GLCIPInstance &instance,
    vector<vector<DNode>> &listOfComponents,
    int nComponents)
{
    vector<double> thr(nComponents);

    //find the minimum threshold on each component
    for (int i = 0; i < nComponents; i++)
    {
        double minThreshold = 1e+20;
        //printf("size of component %d: %ld\n", i, listOfComponents[i].size());
        for (size_t j = 0; j < listOfComponents[i].size(); j++)
        {
            DNode w = listOfComponents[i][j];
            minThreshold = min(minThreshold, instance.threshold[w]);
            //printf("threshold of %s: %f\n", instance.nodeName[w].c_str(), instance.threshold[w]);
        }
        thr[i] = minThreshold;
        //printf("threshold(%d) = %.1f\n", i, minThreshold);
    }
    return thr;
}

// add the condensed arcs in the condensed graph
void getCondensedArcWeights(
    GLCIPInstance &instance,
    Digraph &condensed,
    Digraph &graph,
    ArcArcMap &arcRef,
    ArcValueMap &weights,
    Digraph::NodeMap<int> &components)
{
    //get the cut arcs of the strongly connected components
    Digraph::ArcMap<bool> cutArcs(graph, FALSE);
    stronglyConnectedCutArcs(graph, cutArcs);

    //ArcValueMap condensedInfluence(condensed);
    for (ArcIt a(graph); a != INVALID; ++a)
    {
        //compute the weigh of influence of the arcs
        //each arc receives the total of weights of all arcs from a component to another
        if (cutArcs[a])
        {
            //reference to what component each vertex belong
            int i = components[graph.source(a)];
            int j = components[graph.target(a)];

            Arc b = findArc(condensed, condensed.nodeFromId(i), condensed.nodeFromId(j));
            if (b == INVALID)
            {
                Arc c = condensed.addArc(condensed.nodeFromId(i), condensed.nodeFromId(j));
                weights[c] = instance.influence[arcRef[a]];
            }
            else
                weights[b] += instance.influence[arcRef[a]];
        }
        else
        {
            //reference to what component each vertex belong
            int i = components[graph.source(a)];
            int j = components[graph.target(a)];

            if (i != j)
            {
                cout << "something wrong i != j\n";
            }
        }
    }
}

void printCondensedArcs(Digraph &condensed, ArcValueMap &condensedInfluence)
{
    for (ArcIt a(condensed); a != INVALID; ++a)
    {
        int i = condensed.id(condensed.source(a));
        int j = condensed.id(condensed.target(a));

        printf("condensed arc: %d - > %d: %f", i, j, condensedInfluence[a]);
    }
}

double getMinimumThreshold(GLCIPInstance &instance, DNode &node)
{
    double minimum = 1e+09;
    for (DNodeIt v(instance.g); v != INVALID; ++v)
    {
        //cout << "threshold of " << instance.nodeName[v] << ": " << instance.threshold[v] << endl;
        double thr = instance.threshold[v];
        if (thr < minimum)
        {
            minimum = thr;
            node = v;

            // stop if thr(v) = 1 because there is no smaller threshold
            if (instance.threshold[v] == 1)
                break;
        }
    }

    return minimum;
}

int getIndexOfChepeastIncentive(GLCIPInstance &instance, DNode &node)
{
    int index = 0;
    for (size_t i = 0; i < instance.incentives[node].size(); i++)
    {
        if (instance.incentives[node][i] >= instance.threshold[node])
        {
            //cout << "incentive paid: " << instance.incentives[node][i] << endl;
            index = i;
            break;
        }
    }
    return index;
}

void getSubGraph(
    SCIP *scip,
    GLCIPInstance &instance,
    Digraph &graph,
    DNodeDNodeMap &nodeRef,
    ArcArcMap &arcRef,
    ArcSCIPVarMap &z)
{
    digraphCopy(instance.g, graph).nodeCrossRef(nodeRef).arcCrossRef(arcRef).run();

    for (ArcIt a(graph); a != INVALID; ++a)
    {
        if (SCIPisEQ(scip, SCIPvarGetUbLocal(z[arcRef[a]]), 0))
        {
            //removing arc variables fixed in zero
            graph.erase(a);
        }
    }

    //GraphViewer::ViewGLCIPSupportGraph(instance, graph, "Sub-graph", nodeRef);
}

/**
 * propagate in the topological ordering of condensed graphfor each condensed node, 
 * if the total of influence incident on it is less than the threshold, 
 * pay the difference (needed incentive) find the vertex of smaller threshold or
 * choose the vertices in the component who receives influence of the previous components
 */
double getCostInTopologicalOrdering(
    Digraph &condensed,
    int nComponents,
    vector<double> thr,
    ArcValueMap &arcWeight,
    vector<double> condIncentives)
{

    //linear time algorithm to solve the problem in DAGs
    double total = 0;
    for (int i = 0; i < nComponents; i++)
    {
        double sum = 0;
        for (InArcIt a(condensed, condensed.nodeFromId(i)); a != INVALID; ++a)
        {
            sum += arcWeight[a];
        }

        if (sum < thr[i])
        {
            double p = 0;
            for (double j : condIncentives)
            {
                if (sum + j >= thr[i])
                {
                    p = j;
                    break;
                }
            }

            total += p;
        }
    }

    return total;
}

bool isIntegral(SCIP *scip, GLCIPInstance &instance, DNodeSCIPVarMap &x, ArcSCIPVarMap &z)
{
    bool integral = TRUE;
    for (ArcIt a(instance.g); a != INVALID; ++a)
    {
        if (!SCIPisIntegral(scip, SCIPgetVarSol(scip, z[a])))
        {
            integral = FALSE;
        }
    }
    return integral;
}

SCIP_DECL_RELAXEXEC(DualBound::scip_exec)
{
    SCIP_Real relaxval;

    *result = SCIP_DIDNOTRUN;
    *lowerbound = -SCIPinfinity(scip);

    //get the support graph of the current feasible solution
    Digraph graph;

    DNodeDNodeMap nodeRef(graph); //save the reference to the original node
    ArcArcMap arcRef(graph);      //save the reference to the original arc
    getSubGraph(scip, instance, graph, nodeRef, arcRef, z);

    if (stronglyConnected(graph) || instance.alpha < 1)
    {
        //cout << "assossiated subgraph is strongly connected\n";
        //find minimum threshold vertex
        DNode node = INVALID;
        relaxval = getMinimumThreshold(instance, node);
    }
    else
    {
        int nComponents = countStronglyConnectedComponents(graph);
        /* cout << "assossiated subgraph isn't strongly connected: ";
        cout << nComponents << " components\n"; */

        Digraph condensed;
        vector<vector<DNode>> listOfComponents(nComponents);
        vector<double> thr(nComponents);
        Digraph::NodeMap<int> components(graph);
        ArcValueMap arcWeight(condensed);

        getCondensedGraph(instance, condensed, graph, nodeRef, components, listOfComponents);
        getCondensedArcWeights(instance, condensed, graph, arcRef, arcWeight, components);
        thr = getCondensedThresholds(instance, listOfComponents, nComponents);
        //printCondensedArcs(condensed, arcWeight);

        //just to record the possible incentives (assuming that they are the same for every vertex)
        vector<double> condIncentives;
        for (DNodeIt v(instance.g); v != INVALID; ++v)
        {
            condIncentives = instance.incentives[v];
            break; // only one arbitrary vertex is needed
        }

        //linear time algorithm to solve the problem in DAGs
        relaxval = getCostInTopologicalOrdering(condensed, nComponents, thr, arcWeight, condIncentives);
    }
    *lowerbound = SCIPtransformObj(scip, relaxval);
    *result = SCIP_SUCCESS;

    return SCIP_OKAY;
}