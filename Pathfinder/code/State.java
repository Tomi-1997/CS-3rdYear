/* Represents the current state of a path, by knowing where are we and how much it cost to get here */
package code;
public class State implements Comparable<State>
{
    int i;
    int j;
    int cost;
    int age;
    State prev;

    public State(int i, int j, int cost)
    {
        this.i = i;
        this.j = j;
        this.cost = cost;
        this.prev = null;
        this.age = 0;
    }

    public State(int i, int j, int cost, State prev)
    {
        this.i = i;
        this.j = j;
        this.cost = cost;
        this.prev = prev;
        this.age = 0;
    }

    public String toString()
    {
        // Can't compile on linux
        //        return "(%d, %d, g=%d, f=%d)".formatted(this.i, this.j
        //                , this.cost, code.Board.cost(this));

        return "(" + this.i + ", " + this.j + ", g=" + this.cost + ", f=" + Board.cost(this) + ")";
    }

    @Override
    public int compareTo(State o)
    {
        // Bigger than 0, this state is more expensive
        // 0 - same cost
        // Less than 0, this state is cheaper

        int res =  Board.cost(this) - Board.cost(o);

        // If the cost is the same, return result by preference for older / newer states.
        if (res == 0)
            return (this.age - o.age) * Pathfinder.AGE_PREF;
                    // If this node is older, the difference is positive, times 1 if older nodes are preferred- this node will rank higher.

        return res;
    }
}
