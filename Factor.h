/** Factor.h --- 
 *
 * Copyright (C) 2016 Zhen Zhang
 *
 * Author: Zhen Zhang <zhen@zzhang.org>
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef FACTOR_H
#define FACTOR_H 1

#include <iomanip>
#include <utility>
#include <unordered_map>
#include <vector>
#include "PRTypes.h"
#include <iostream>
#include <cstring>


#define FACTOR_INVALID -1
#define FACTOR_NODE_ID 1
#define FACTOR_EDGE_ID 2
#define FACTOR_SPARSEEDGE_ID 3
#define FACTOR_GENERAL_ID 4
#define FACTOR_GENERALSPARSE_ID 5



struct EdgeInternal{
     int ei;
     int ej;
     Real* data;
};
struct SparseEdgeInternal{
     int ei;
     int ej;
     Real *data;
     Real *mi;
     Real *mj;
     int nnz;
     int *nnzIdx;
};


namespace zzhang{
     /**
      * Base class for a factor.
      */
     class CFactorBase{
     public:
	  /**
	   * A public interface to create a factor.
	   */
	  typedef CFactorBase* (*FactorCreator)(const std::vector<int>& nodes,
						void *data);

	       
	  /**
	   * Register a factor creator.
	   */
	  static bool RegisterFactorCreator(int ID, FactorCreator creator){
	       if(ID == FACTOR_INVALID) return false;
	       if(FactorCreators.find(ID) != FactorCreators.end())
		    return false;
	       FactorCreators[ID] = creator;
	       return true;
	  }

     public:

	  /**
	   * Create a factor. It should only be called at 
	   */ 
	  static CFactorBase* CreateFactor(int ID, const std::vector<int>& nodes,
					   void *data){
	       if(ID == FACTOR_INVALID) return NULL;
	       if(FactorCreators.find(ID) == FactorCreators.end())
		    return NULL;
	       FactorCreator creator = FactorCreators[ID];
	       return creator(nodes, data);
	  }
	  CFactorBase(){
	       m_LocalMax = 0;
	  }
     public:
	  int m_LocalMax;
	  
     public:
	  /**
	   * return the primal value of current factor with given decode.
	   * @param decode given decode
	   */
	  virtual Real Primal(int *decode) = 0;
	  /**
	   * return current dual.
	   */
	  virtual Real Dual() = 0;
	  /**
	   * Updating all message variables.
	   */
	  virtual void UpdateMessages() = 0;
	  /**
	   * Print instrinsic information of the factor.
	   */
	  virtual void Print() = 0;
	  virtual bool IsGeneralFactor() = 0;
	  virtual bool GetIncludedNodes(std::vector<int>& nodes) = 0;
	  int size()
	  {
	       std::vector<int> nodes;
	       GetIncludedNodes(nodes);
	       return nodes.size();
	  }
	  /**
	   * Desctrotor;
	   */
	  virtual ~CFactorBase() {};
     private:
	  static std::unordered_map<int, FactorCreator > FactorCreators;
     public:
	  static const int FactorID = FACTOR_INVALID;
     };
     class CFactorGraph;

     
     class NodeFactor : public CFactorBase
     {
     public:
	  Real * m_bi;
	  int m_NofStates;
	  int m_id;
     public:
	  NodeFactor(){
	       m_bi = NULL;
	       m_NofStates = 0;
	       m_id = -1;
	  }
	  friend class CFactorGraph;
     private:
	  /**
	   * Creator, can only be called from CFactorGraph
	   */
	  NodeFactor(int id, int NofStates, Real* bi){
	       m_id = id;
	       m_bi = bi;
	       m_NofStates = NofStates;
	       m_LocalMax = 0;
	  }
     public:
	  virtual ~NodeFactor(){
	  }
	  virtual void UpdateDual(){
	  }
	  virtual Real Primal(int *decode){
	       return m_bi[decode[m_id]];
	  }
	  virtual Real Dual(){
	       return m_bi[m_LocalMax];
	  }
	  virtual void UpdateMessages(){
	  }
	  virtual bool IsGeneralFactor(){
	       return true;
	  }
	  virtual bool GetIncludedNodes(std::vector<int>& nodes){
	       nodes = std::vector<int>(1);
	       nodes[0] = m_id;
	  }
	  virtual void Print(){
	       std::cout << "Node " << m_id;
	       std::cout << " NofStates: " << m_NofStates;
	       std::cout << " LocalMax: " << m_LocalMax << std::endl;
	       std::cout << " Potential : " << std::endl;
	       for(int xi = 0; xi < m_NofStates; xi++)
	       {
		    std::cout << m_bi[xi] << " "; 
	       }
	       std::cout << std::endl;
	  }

     };
     
     class DenseEdgeFactor : public CFactorBase
     {
     private:
	  Real* bi;
	  Real* bj;
	  Real* bij;
	  int ei;
	  int ej;
	  int *NofStates;
	  NodeFactor *n1;
	  NodeFactor *n2;
     private:
	  friend class CFactorGraph;
	  DenseEdgeFactor(const void* InParam, const ExternalData* OuParam);
     public:
	  virtual Real Primal(int *decode){
	       return bij[decode[ei] * NofStates[ej] + decode[ej]];
	  }
	  virtual Real Dual(){
	       return bij[m_LocalMax];
	  }
	  virtual bool IsGeneralFactor(){
	       return true;
	  }
	  virtual void UpdateMessages(){
	       Real LocalMaxV = -10000;
	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    int base = xi * NofStates[ej];
		    for(int xj = 0; xj < NofStates[ej]; xj++)
		    {
			 bij[base++] += bi[xi] + bj[xj];
		    }
	       }
	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    bi[xi] = -10000;
	       }
	       for(int xj = 0; xj < NofStates[ej]; xj++)
	       {
		    bj[xj] = -10000;
	       }

	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    int base = xi * NofStates[ej];
		    for(int xj = 0; xj < NofStates[ej]; xj++)
		    {
			 if(bij[base] > bi[xi])
			      bi[xi] = bij[base];
			 if(bij[base] > bj[xj])
			      bj[xj] = bij[base];
			 if(bij[base] > LocalMaxV)
			 {
			      LocalMaxV = bij[base];
			      m_LocalMax = base;
			      n1->m_LocalMax = xi;
			      n2->m_LocalMax = xj;
			 }
			 base++;
		    }
	       }

	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    bi[xi] *= 0.5;
	       }
	       for(int xj = 0; xj < NofStates[ej]; xj++)
	       {
		    bj[xj] *= 0.5;
	       }

	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    int base = xi * NofStates[ej];
		    for(int xj = 0; xj < NofStates[ej]; xj++)
		    {
			 bij[base++] -= bi[xi] + bj[xj];
		    }
	       }
	  }
	  virtual bool GetIncludedNodes(std::vector<int>& nodes) {
	       nodes =std::vector<int>(2);
	       nodes[0] = ei; nodes[1] = ej;
	  }
	  
	  virtual void Print(){
	       int xijMax = NofStates[ei] * NofStates[ej];
	       std::cout << "Edge: " << ei << " " << ej << std::endl;
	       std::cout << "Potentials : " << NofStates[ei] << "x" << NofStates[ej] <<  " LocalMax: " << m_LocalMax <<std::endl;
	       for(int xi = 0; xi < NofStates[ei]; xi++)
	       {
		    int base = xi * NofStates[ej];
		    for(int xj = 0; xj < NofStates[ej]; xj++)
		    {
			 std::cout << std::showpoint <<  std::setprecision(6) << std::setw(10) <<bij[base++] << " ";
		    }
		    std::cout << std::endl;
	       }
	       
	       
	  }
	  virtual ~DenseEdgeFactor(){
	       delete []bij;
	  }
     public:
	  static const int FactorID = FACTOR_EDGE_ID;

     };
}

#endif // FACTOR_H
