#pragma once
/* 
 * class definition of graph
 */
// #include "ms_info_head.h"
#include <stdlib.h>
#include <iostream>
#include <stddef.h>		// for NULL
#include <limits>
/* edge in the graph definition */
using namespace std;

template<class dist_type>
class edge
{
public:
	edge(int dest, dist_type weight)
	{
		dest_node = dest;
		dist_weight = weight;
		next_edge = NULL;
	}
	~edge()
	{

	}
public:
	int dest_node;
	dist_type dist_weight;
	edge<dist_type>  *next_edge;
};

/* vertex in the graph definition*/
template<class node_type, class dist_type> class graph;
template<class node_type, class dist_type>
class vertex
{
public:
	vertex()
	{
		padj_edge = NULL;
		vertex_name = 0;
	}
	~vertex()
	{

	}
public:
	friend class graph<node_type, dist_type>;
	node_type vertex_name;
	edge<dist_type> *padj_edge;
};

/* graph definition */
template<class node_type, class dist_type>
class graph
{
public:
	graph(int size = default_size)
	{
		vertex_table = new vertex<node_type, dist_type>[size];
		if (vertex_table == NULL)
		{
			exit(1);
		}
		num_vertex = 0;  
        max_size = size;  
        num_edges = 0;  
	}

	~graph()  
    {  
        edge<dist_type> *pmove;  
        for (int i = 0; i < this->num_vertex; i++)  
        {  
            pmove = this->vertex_table[i].padj_edge;  
            if (pmove){  
                this->vertex_table[i].padj_edge = pmove->next_edge;  
                delete pmove;  
                pmove = this->vertex_table[i].padj_edge;  
            }  
        }  
        delete[] vertex_table;  
    }  

    int get_edge_num()  
    {  
        return num_edges / 2;  
    }  
    int get_vertex_num()  
    {  
        return num_vertex;  
    }  
    bool is_graph_full() const  
    {   
        return max_size == num_vertex;  
    }  
    
    bool insert_edge(int v1, int v2, dist_type weight = std::numeric_limits<dist_type>::infinity());  	// the default maximum value
    bool insert_vertex(const node_type vertex);   
    void print_graph();  

public:  
    vertex<node_type, dist_type> *vertex_table;    
    int num_vertex; 
    int max_size;   
    static const int default_size = 1000;        
    // static const dist_type infinity = 65536;  //边的默认权值（可以看成是无穷大）  
    int num_edges;
    // int get_vertex_pos(const node_type vertex);		// find pos in table by name
};

/* function definition */
template<class node_type, class dist_type>
bool graph<node_type, dist_type>::insert_vertex(const node_type vertex_name)
{
	if (is_graph_full())
	{
		cerr << "GRAPH IS FULL" << endl;
		return false;
	}
	else
	{
		this->vertex_table[this->num_vertex].vertex_name = vertex_name;
		this->num_vertex++;
		return true;
	}
	return true;
};

template<class node_type, class dist_type>
bool graph<node_type, dist_type>::insert_edge(int v1, int v2, dist_type weight)
{
	if (v1 < 0 || v1 > num_vertex || v2 < 0 || v2 > num_vertex)
	{
		cerr << "WRONG POSITION!" << endl;
		return false;
	}
	else
	{
		edge<dist_type> *pmove = vertex_table[v1].padj_edge;
		if (pmove == NULL)
		{
			/* construct the first edge of this node */
			vertex_table[v1].padj_edge = new edge<dist_type>(v2, weight);
			num_edges++;
			return true;
		}
		else
		{
			while(pmove->next_edge)
			{
				/* 当v2已经存在，出现在前四个峰有一个以上重合的情况下 */
				if (pmove->dest_node == v2)
				{
					break;
				}
				pmove = pmove->next_edge;
			}
			if (! pmove->next_edge)
			{
				pmove->next_edge = new edge<dist_type>(v2,weight);
				num_edges++;
				return true;
			}	
		}
	}
	return true;
};