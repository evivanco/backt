import java.util.*;

public class ParallelShortestPathSolver {
    private final Graph graph;
    private final int target;

    public ParallelShortestPathSolver(Graph graph, int target) {
        this.graph = graph;
        this.target = target;
    }

    public PathResult solve(int start) throws InterruptedException {
        BestPathShared sharedBest = new BestPathShared();
        List<Thread> threads = new ArrayList<>();

        for (Edge edge : graph.getNeighbors(start)) {
            List<Integer> path = new ArrayList<>();
            Set<Integer> visited = new HashSet<>();

            path.add(start);
            visited.add(start);

            ParallelShortestPathWorker worker = new ParallelShortestPathWorker(
                    graph,
                    edge.to,
                    target,
                    edge.weight,
                    path,
                    visited,
                    sharedBest
            );

            threads.add(worker);
            worker.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }

        return sharedBest.getBestResult();
    }
}