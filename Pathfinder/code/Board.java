/* Object to hold array of tiles, perform operators on states and calculate costs. */
package code;
import java.util.ArrayList;
import java.util.HashMap;

public class Board
{
    ArrayList<Operator> ops;
    int start_i, start_j;

    static HashMap<Character, Integer> costOf;
    static Tile[][] myBoard;
    static int N, goal_i, goal_j;


    public Board()
    {
        costOf = new HashMap<>();
        costOf.put('D', 1);
        costOf.put('R', 3);
        costOf.put('H', 5);
        costOf.put('X', -1);
        costOf.put('S', 0);
        costOf.put('G', 5);
    }


    public static HashMap<Character, Integer> countPrevVisits(State s)
    {
        /* NULL <- A <- B <- C <- D <- CURR, (<- indicates the previous state.)
        * Counts visit amount for each type of state: a,b,c up to curr.*/
        HashMap<Character, Integer> visitCount = new HashMap<>();
        for (Character c : new Character[]{'D', 'R', 'H'})
            visitCount.put(c, 0);

        State temp = s;
        while (temp != null)
        {
            char t = myBoard[temp.i][temp.j].type;

            if (!visitCount.containsKey(t))         // Ignore S or G
                return visitCount;

            int count = visitCount.get(t);
            visitCount.put(t, count + 1);           // Update tile type visit count
            temp = temp.prev;
        }
        return visitCount;
    }


    public static int cost(State s)
    {
        HashMap<Character, Integer> visitCount = countPrevVisits(s);
        // Actual cost + approximation of cost to reach goal from here
        return g(s) + h(s, visitCount);
    }


    public static int g(State s)
    {
        // Actual cost
        return s.cost;
    }


    public static int h(State s, HashMap<Character, Integer> visitCount)
    {
        // Optimistic heuristic function
        //
        // Use L2 distance as diagonal movement is allowed.
        // If there are any dirt tiles left on map, assume it is on the way to the goal
        // else, if there is any road left, assume we will be on the road
        // else- just sand left.
        int res = 0;
        int overallTiles = N * N - 2; // 2 present tiles are always start and goal.
        double maxDirtPer = 0.1;
        double maxRoadPer = 0.1;

        int maxDirt = (int) (overallTiles * maxDirtPer);
        int maxRoad = (int) (overallTiles * maxRoadPer);

        int dirtLeft = maxDirt - visitCount.get('D');
        int roadLeft = maxRoad - visitCount.get('R');
        int tilesToGoal = (s.i - goal_i)*(s.i - goal_i) + (s.j - goal_j)*(s.j - goal_j);
        tilesToGoal = (int) (Math.sqrt(tilesToGoal) + 0.5); // round up

        int d = costOf.get('D');
        int r = costOf.get('R');
        int h = costOf.get('H');

        /* Approximate path cost to goal */
        while (tilesToGoal > 0)
        {
            if (dirtLeft > 0)
            {
                res += d;
                dirtLeft--;
            }
            else if (roadLeft > 0)
            {
                res += r;
                roadLeft--;
            }
            else
            {
                res += h;
            }
            tilesToGoal--;
        }

        return res;
    }


    public boolean unsafe(int i, int j)
    {
        // Unsafe if i or j are out of bounds
        return i < 0 || i >= N || j < 0 || j >= N;
    }


    public boolean illegal(Operator o, State s)
    {
        /* Out of bounds */
        if (unsafe(o.iInc + s.i, o.jInc + s.j))
            return true;

        /* Next location is a cliff */
        if (tileCost(s.i, s.j, o.iInc + s.i, o.jInc + s.j) == -1)
            return true;

        /* Last check - Applying operator results in the same state */
        return s.prev != null && o.iInc + s.i == s.prev.i && o.jInc + s.j == s.prev.j;
    }


    public int tileCost(int srcI, int srcJ, int i, int j)
    {
        Tile target = myBoard[i][j];

        int ans = costOf.get(target.type);

        /* Check if diagonal and target is sand, if so- add 5 */
        boolean sandDiagonal = (target.type == 'H') && (srcI != i && srcJ != j);
        if (sandDiagonal)
            ans += 5;

        return ans;
    }


    public void initOps(String dir)
    {
        this.ops = new ArrayList<>();

        /* Get pathing to produce operators by */
        /* Clockwise example, (i, j) is the increase to the i and j of the operated node.
           (-1,-1) (-1,0) (-1, 1)
           (0, -1)    n   ( 0, 1)
           (-1,-1) ( 1,0) ( 1, 1)
        * */
        int[] iIndex = {0, 1, 1, 1, 0, -1, -1, -1};
        int[] jIndex = {1, 1, 0, -1, -1, -1, 0, 1};

        /* This rotation fits to 3x3 matrices only. */
        if (dir.compareTo("counter-clockwise") == 0)
        {
            iIndex = new int[]{0, -1, -1, -1, 0, 1, 1, 1};
            jIndex = new int[]{1, 1, 0, -1, -1, -1, 0, 1};
        }

        for (int k = 0; k < iIndex.length; k++)
        {
            /* Clock index array to i,j indexes */
            int i = iIndex[k];
            int j = jIndex[k];

            Operator o = new Operator(i, j);
            ops.add(o);
        }
    }


    public void initBoard(String[] input)
    {
        N = input.length;
        myBoard = new Tile[N][N];

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                char c = input[i].charAt(j);
                int t = costOf.get(c);
                myBoard[i][j] = new Tile(c, t);
            }
        }
    }


    public State operate(Operator o, State s)
    {
        int i = o.iInc + s.i;
        int j = o.jInc + s.j;
        int c = s.cost + tileCost(s.i, s.j, i, j);

        return new State(i, j, c, s);
    }


    public String toString()
    {
        StringBuilder ans = new StringBuilder();
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                ans.append(myBoard[i][j].type);
                ans.append(" ");
            }
            ans.append('\n');
        }
        return ans.toString();
    }


    public boolean isGoal(State s)
    {
        return (s.i == goal_i && s.j == goal_j);
    }

}
