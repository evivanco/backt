import java.util.*;

public class Graph {
    private final Map<Integer, List<Edge>> adjacencyList = new HashMap<>();

    public void addNode(int node) {
        adjacencyList.putIfAbsent(node, new ArrayList<>());
    }

    public void addEdge(int from, int to, int weight) {
        addNode(from);
        addNode(to);
        adjacencyList.get(from).add(new Edge(to, weight));
    }

    public List<Edge> getNeighbors(int node) {
        return adjacencyList.getOrDefault(node, new ArrayList<>());
    }
}