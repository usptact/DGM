#include "DecodeTRW.h"
#include "Graph.h"
#include "TRW_S\MRFEnergy.h"
#include "macroses.h"

namespace DirectGraphicalModels
{
vec_byte_t CDecodeTRW::decode(unsigned int nIt, Mat &lossMatrix) const
{
	byte			nStates	= m_pGraph->m_nStates;					// number of states (classes)
	size_t			nNodes	= m_pGraph->getNumNodes();				// number of nodes
	vec_byte_t		res(nNodes);

	DGM_IF_WARNING(!lossMatrix.empty(), "The Loss Matrix is not supported by the algorithm.");
	
	MRFEnergy			* mrf	= new MRFEnergy(nStates);
	MRFEnergy::Node*	* nodes = new MRFEnergy::Node*[nNodes];
	double				* nPot	= new double[nStates];
	double				* ePot	= new double[nStates * nStates];

	// Add Nodes
	for (Node &node : m_pGraph->m_vNodes) {
		for (byte s = 0; s < nStates; s++) nPot[s] = -logf(MAX(FLT_EPSILON, node.Pot.at<float>(s, 0)));
		nodes[node.id] = mrf->AddNode(nPot);
	}

	// Add edges
	for (Edge &edge : m_pGraph->m_vEdges) 
		if (edge.node2 > edge.node1) {
			int k = 0;
			for (byte i = 0; i < nStates; i++)
				for (byte j = 0; j < nStates; j++)
					ePot[k++] = -logf(MAX(FLT_EPSILON, edge.Pot.at<float>(j, i)));

			mrf->AddEdge(nodes[edge.node1], nodes[edge.node2], ePot);
		}
	

	/////////////////////// TRW-S algorithm //////////////////////
	MRFEnergy::Options	  options;
	double				  energy;
	double				  lowerBound;

	options.m_iterMax = nIt; // maximum number of iterations
	mrf->Minimize_TRW_S(options, lowerBound, energy);

	// read solution
	for (size_t n = 0; n < nNodes; n++)		
		res[n] = static_cast<byte>(nodes[n]->m_solution);

	// done
	delete mrf;
	delete nodes;
	delete nPot;
	delete ePot;
	
	return res;
}
}



