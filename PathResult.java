import java.util.*;

public class PathResult {
    private final List<Integer> path;
    private final int cost;

    public PathResult(List<Integer> path, int cost) {
        this.path = path;
        this.cost = cost;
    }

    public List<Integer> getPath() {
        return path;
    }

    public int getCost() {
        return cost;
    }

    @Override
    public String toString() {
        return "Camino: " + path + " | Costo: " + cost;
    }
}