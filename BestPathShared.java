public class BestPathShared {
    private PathResult bestResult = null;

    public synchronized void updateBest(PathResult candidate) {
        if (candidate == null) return;

        if (bestResult == null || candidate.getCost() < bestResult.getCost()) {
            bestResult = candidate;
        }
    }

    public synchronized PathResult getBestResult() {
        return bestResult;
    }
}