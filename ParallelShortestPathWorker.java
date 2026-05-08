import java.util.*;

public class ParallelShortestPathWorker extends Thread {
    private final Graph graph;
    private final int current;
    private final int target;
    private final int currentCost;
    private final List<Integer> path;
    private final Set<Integer> visited;
    private final BestPathShared sharedBest;

    public ParallelShortestPathWorker(
            Graph graph,
            int current,
            int target,
            int currentCost,
            List<Integer> path,
            Set<Integer> visited,
            BestPathShared sharedBest
    ) {
        this.graph = graph;
        this.current = current;
        this.target = target;
        this.currentCost = currentCost;
        this.path = path;
        this.visited = visited;
        this.sharedBest = sharedBest;
    }

    @Override
    public void run() {
        backtrack(current, currentCost, path, visited);
    }

    private void backtrack(int node, int cost, List<Integer> currentPath, Set<Integer> currentVisited) {
        currentPath.add(node);
        currentVisited.add(node);

        if (node == target) {
            PathResult result = new PathResult(new ArrayList<>(currentPath), cost);
            sharedBest.updateBest(result);
        } else {
            for (Edge edge : graph.getNeighbors(node)) {
                if (!currentVisited.contains(edge.to)) {
                    backtrack(
                            edge.to,
                            cost + edge.weight,
                            currentPath,
                            currentVisited
                    );
                }
            }
        }

        currentPath.remove(currentPath.size() - 1);
        currentVisited.remove(node);
    }
}