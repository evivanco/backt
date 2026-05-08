public class ClassTests {
    public static void main(String[] args) throws InterruptedException {
        Graph graph = new Graph();

        graph.addEdge(0, 1, 4);
        graph.addEdge(0, 2, 2);
        graph.addEdge(1, 3, 5);
        graph.addEdge(2, 3, 1);

        SequentialShortestPathSolver sequential = new SequentialShortestPathSolver(graph, 3);
        PathResult sequentialResult = sequential.solve(0);

        System.out.println("Resultado secuencial:");
        System.out.println(sequentialResult);

        ParallelShortestPathSolver parallel = new ParallelShortestPathSolver(graph, 3);
        PathResult parallelResult = parallel.solve(0);

        System.out.println("Resultado paralelo:");
        System.out.println(parallelResult);
    }
}