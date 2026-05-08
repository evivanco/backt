import java.util.*;

public class SequentialShortestPathSolver {
    private final Graph graph;
    private final int target;

    private PathResult bestResult = null;

    public SequentialShortestPathSolver(Graph graph, int target) {
        this.graph = graph;
        this.target = target;
    }

    public PathResult solve(int start) {
        List<Integer> path = new ArrayList<>();
        Set<Integer> visited = new HashSet<>();

        backtrack(start, 0, path, visited);

        return bestResult;
    }

    private void backtrack(int current, int currentCost, List<Integer> path, Set<Integer> visited) {
        path.add(current);
        visited.add(current);

        if (current == target) {
            if (bestResult == null || currentCost < bestResult.getCost()) {
                bestResult = new PathResult(new ArrayList<>(path), currentCost);
            }
        } else {
            for (Edge edge : graph.getNeighbors(current)) {
                if (!visited.contains(edge.to)) {
                    backtrack(edge.to, currentCost + edge.weight, path, visited);
                }
            }
        }

        path.remove(path.size() - 1);
        visited.remove(current);
    }
}