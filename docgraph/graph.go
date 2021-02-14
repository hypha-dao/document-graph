package docgraph

import (
	"context"
	"fmt"
	"log"
	"sync"

	eos "github.com/eoscanada/eos-go"
)

var (
	documents []Document
	edges     []Edge
	nodes     map[string]*Node
)

// Graph ...
type Graph struct {
	Nodes []*Node
	Edges []*Edge
	lock  sync.RWMutex
}

// Node ...
type Node struct {
	document      *Document
	OutboundEdges map[eos.Name][]*Node
	InboundEdges  map[eos.Name][]*Node
}

func (n *Node) getLabel() string {
	return n.document.GetNodeLabel()
}

// AddNode ...
func (g *Graph) AddNode(n *Node) {
	log.Println("Adding node --- " + n.getLabel())
	g.lock.Lock()
	g.Nodes = append(g.Nodes, n)
	nodes[n.document.Hash.String()] = n
	g.lock.Unlock()
}

// Connect adds an edge to the graph
func (g *Graph) Connect(n1, n2 *Node, edgeName eos.Name) {
	log.Println("Connecting ---- " + n1.getLabel() + "--- " + string(edgeName) + " ---> " + n2.getLabel())
	g.lock.Lock()

	if n1.OutboundEdges == nil {
		n1.OutboundEdges = make(map[eos.Name][]*Node)
	}

	if n2.InboundEdges == nil {
		n2.InboundEdges = make(map[eos.Name][]*Node)
	}

	n1.OutboundEdges[edgeName] = append(n1.OutboundEdges[edgeName], n2)
	n2.InboundEdges[edgeName] = append(n2.InboundEdges[edgeName], n1)

	g.lock.Unlock()
}

// LoadGraph ...
func LoadGraph(ctx context.Context, api *eos.API, contract eos.AccountName) (*Graph, error) {

	graph := Graph{}

	edges, err := GetAllEdges(ctx, api, contract)
	if err != nil {
		return &graph, fmt.Errorf("cannot get all edges %v", err)
	}

	documents, err := GetAllDocuments(ctx, api, contract)
	if err != nil {
		return &graph, fmt.Errorf("cannot get all documents %v", err)
	}

	graph.Nodes = make([]*Node, len(documents))
	graph.Edges = make([]*Edge, len(edges))
	nodes = make(map[string]*Node)

	for _, doc := range documents {
		n := Node{document: &doc}
		graph.AddNode(&n)
	}

	for _, edge := range edges {
		graph.Connect(nodes[edge.FromNode.String()], nodes[edge.ToNode.String()], edge.EdgeName)
	}
	return &graph, nil
}
