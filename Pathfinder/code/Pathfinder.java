/* Performs one of BFS, DFID, A*, IDA*, DFBnB on a given board with various settings such as:
* - Preference to recently created states
* - Moving clockwise in the board
 */
package code;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

public class Pathfinder
{
    String algo, clockDir, timeOption, listPrintOption;
    Board b;
    int nodesCreated;
    int finalCost;
    static int AGE_PREF;
    boolean printOpen = false;

    public static final String FAIL = "no path";
    HashSet<String> solution;

    public Pathfinder(String whichAlgo, String clockDir, String timeOption, String listPrintOption, Board b)
    {
        this.algo = whichAlgo;
        this.clockDir = clockDir;
        this.timeOption = timeOption;
        this.listPrintOption = listPrintOption;
        this.b = b;
        this.solution = new HashSet<>();

        if (listPrintOption.compareTo("with open") == 0)
            this.printOpen = true;
    }


    public void setup()
    {
        finalCost = 0;
        nodesCreated = 0;
    }


    /* Primarily for debugging */
    public void all()
    {
        String decor = "----";
        this.algo = "bfs";
        System.out.println(decor + "BFS" + decor);
        this.run();
        setup();
        this.algo = "dfid";
        System.out.println(decor + "DFID" + decor);
        this.run();
        setup();
        this.algo = "a*";
        System.out.println(decor + "A*" + decor);
        this.run();
        setup();
        this.algo = "ida*";
        System.out.println(decor + "IDA*" + decor);
        this.run();
        setup();
        this.algo = "dfbnb";
        System.out.println(decor + "DFBnb" + decor);
        this.run();
    }


    public String run()
    {
        double start = System.currentTimeMillis();
        String info = "";
        switch (this.algo.toLowerCase())
        {
            case "bfs":
                info = this.run_bfs();
                break;

            case "dfid":
                info = this.run_dfid();
                break;

            case "a*":
                info = this.run_AStar();
                break;

            case "ida*":
                info = this.run_IDAStar();
                break;

            case "dfbnb":
                info = this.run_DFBnB();
                break;
        }
        double end = System.currentTimeMillis();

        boolean fail = info.compareTo(FAIL) == 0;
        info += "\n" + "Num: " + nodesCreated;

        if (fail)
            info += "\nCost: inf\n";
        else
            info += "\n" + "Cost: "+ finalCost + "\n";

        double secondsPassed = (end - start) / 1000;

        if (timeOption.toLowerCase().compareTo("with time") == 0)
            info += String.format("%.3f seconds", secondsPassed) + "\n";

        /* Write to file */
        try
        {
            FileWriter myWriter = new FileWriter("output.txt");
            myWriter.write(info);
            myWriter.close();
        }
        catch (IOException e)
        {
            System.out.println("An error occurred.");
            e.printStackTrace();
        }
        return info;
    }


    public String path(State curr)
    {
        StringBuilder ans = new StringBuilder();
        this.finalCost = curr.cost;
        ArrayList<String> res =  path_(curr, new ArrayList<>());

        for (int i = res.size() - 1; i >= 0; i--)
        {
            ans.append(res.get(i)).append("-");
        }

        /* */
        if (ans.length() == 0)
            return "-";

        /* Remove last '-' */
        ans = new StringBuilder(ans.substring(0, ans.length() - 1));
        return ans.toString();
    }


    public ArrayList<String> path_(State curr, ArrayList<String> ans)
    {
        if (curr.prev == null)
        {
            return ans;
        }

        solution.add(key(curr));

        String stepTaken = "";
        State prev = curr.prev;

        if (prev.i == curr.i + 1)
        {
            stepTaken = stepTaken + "U";
        }

        if (prev.i == curr.i - 1)
        {
            stepTaken = stepTaken + "D";
        }

        if (prev.j == curr.j + 1)
        {
            stepTaken = "L" + stepTaken;
        }

        if (prev.j == curr.j - 1)
        {
            stepTaken = "R" + stepTaken;
        }

        ans.add(stepTaken);
        return path_(curr.prev, ans);
    }


    public String run_bfs()
    {
        State curr = new State(b.start_i, b.start_j,0);
        nodesCreated++;

        Queue<State> queue = new LinkedList<>();
        HashMap<String, State> open_list = new HashMap<>();   // Nodes yet to discover, in discovery queue
        HashMap<String, State> closed_list = new HashMap<>(); // Nodes discovered and expanded

        queue.add(curr);
        open_list.put( key(curr.i, curr.j), curr );

        while (queue.size() > 0)
        {
            if (this.printOpen)
                System.out.println(open_list.values());

            curr = queue.remove();
            open_list.remove( key(curr.i, curr.j) );
            closed_list.put( key(curr.i, curr.j), curr);

            for (Operator o : b.ops)
            {
                if (b.illegal(o, curr))
                    continue;

                State s = b.operate(o, curr);
                s.age = nodesCreated;
                nodesCreated++;
                String sp = key(s.i, s.j);

                if (!open_list.containsKey(sp) && !closed_list.containsKey(sp))
                {
                    if (b.isGoal(s))
                    {
                        return path(s);
                    }
                    queue.add(s);
                    open_list.put(sp, s);
                }

            }
        }
        return FAIL;
    }


    private String key(int i, int j)
    {
        //
        return i+","+j;
    }


    private String key(State s)
    {
        //
        return key(s.i, s.j);
    }


    public String run_dfid()
    {
        State start = new State(b.start_i, b.start_j,0);
        nodesCreated++;

        for (int depth = 1 ; ; depth++)
        {
            HashMap<String, State> H = new HashMap<>();
            String result = Limited_DFS(start, depth, H);
            if (result.compareTo("cutoff") != 0) return result;
        }
    }


    public String Limited_DFS(State curr, int limit, HashMap<String, State> H)
    {
        if (b.isGoal(curr)) return path(curr);
        if (limit == 0) return "cutoff";

        H.put(key(curr), curr);
        if (this.printOpen)
            System.out.println(H.values());

        boolean cut = false;
        for (Operator o : b.ops)
        {
            if (b.illegal(o, curr))
                continue;

            if (H.containsKey(key(curr.i + o.iInc, curr.j + o.jInc)))
                continue;

            State s = b.operate(o, curr);
            s.age = nodesCreated;
            nodesCreated++;

            String result = Limited_DFS(s, limit - 1, H);

            if (result.compareTo("cutoff") == 0)
                cut = true;

            else if (result.compareTo(FAIL) != 0)
                return result;
        }

        H.remove(key(curr));
        if (cut)
            return "cutoff";

        return FAIL;
    }


    public String run_AStar()
    {
        State curr = new State(b.start_i, b.start_j,0);
        nodesCreated++;

        Queue<State> queue = new PriorityQueue<>();   // Sort elements by cost
        HashMap<String, State> open_list = new HashMap<>();   // Nodes yet to discover, in discovery queue
        HashMap<String, State> closed_list = new HashMap<>(); // Nodes discovered, if found again - ignore.

        queue.add(curr);
        open_list.put( key(curr.i, curr.j), curr);

        while (queue.size() > 0)
        {
            curr = queue.remove();
            String p = key(curr.i, curr.j);


            if (this.printOpen)
                System.out.println(open_list.values());
            open_list.remove(p);

            if (b.isGoal(curr))
                return path(curr);

            closed_list.put(p, curr);
            for (Operator o : b.ops)
            {
                if (b.illegal(o, curr))
                    continue;

                State s = b.operate(o, curr);
                s.age = nodesCreated;
                nodesCreated++;

                String sp = key(s.i, s.j);

                /* Haven't been here before, add to list */
                if (!open_list.containsKey(sp) && !closed_list.containsKey(sp))
                {
                    queue.add(s);
                    open_list.put(sp, s);
                }
                /* If node already exists with a higher price- replace it */
                else if (open_list.containsKey(sp) && open_list.get(sp).compareTo(s) > 0)
                {
                    queue.remove(open_list.get(sp));
                    queue.add(s);
                    open_list.put(sp, s);

                }
            }
        }
        return FAIL;
    }


    public String run_IDAStar()
    {
        Stack<State> L = new Stack<>();
        HashMap<String, State> H = new HashMap<>();
        State start = new State(b.start_i, b.start_j,0);
        nodesCreated++;
        int thresh = Board.cost(start);

        while (thresh != Integer.MAX_VALUE)
        {
            HashSet<State> isOut = new HashSet<>();
            int minF = Integer.MAX_VALUE;
            L.push(start);
            H.put(key(start), start);

            while (L.size() > 0)
            {
                State curr = L.pop();
                String currKey = key(curr);

                if (this.printOpen)
                    System.out.println(H.values());

                if (isOut.contains(curr))
                {
                    H.remove(currKey);
                    continue;
                }

                isOut.add(curr);
                L.push(curr);

                for (Operator o : b.ops)
                {
                    if (b.illegal(o, curr))
                        continue;

                    State s = b.operate(o, curr);
                    s.age = nodesCreated;
                    nodesCreated++;

                    int f = Board.cost(s);
                    String sKey = key(s);
                    if (f > thresh)
                    {
                        if (f < minF)
                        {
                            minF = f;
                        }
                        continue;
                    }

                    if (H.containsKey(sKey) && isOut.contains(s))
                    {
                        continue;
                    }

                    if (H.containsKey(sKey) && !isOut.contains(s))
                    {
                        State sTag = H.get(sKey);
                        if (sTag.cost > s.cost)
                        {
                            L.remove(sTag);
                            H.remove(sKey);
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if (b.isGoal(s))
                        return path(s);

                    L.push(s);
                    H.put(sKey, s);
                }
            }
            thresh = minF;
        }
        return FAIL;
    }


    public String run_DFBnB()
    {
        State start = new State(b.start_i, b.start_j,0);
        nodesCreated++;

        Stack<State> L = new Stack<>();
        L.push(start);

        HashSet<State> isOut = new HashSet<>();
        HashMap<String, State> H = new HashMap<>();
        H.put(key(start), start);

        int thresh = Integer.MAX_VALUE;
        State ans = null;
        while (L.size() > 0)
        {
            State n = L.pop();
            String key = key(n);

            if (this.printOpen)
                System.out.println(H.values());

            if (isOut.contains(n))
            {
                H.remove(key);
                continue;
            }

            isOut.add(n);
            L.push(n);

            ArrayList<State> neighbors = new ArrayList<>();
            for (Operator o : b.ops)
            {
                if (b.illegal(o, n))
                    continue;

                State s = b.operate(o, n);
                nodesCreated++;
                s.age = nodesCreated;
                neighbors.add(s);
            }

            Collections.sort(neighbors);
            Iterator<State> it = neighbors.iterator();
            /* If we find g s.t f(g) > t, then remove g and elements after him*/
            boolean removeAll = false;
            while(it.hasNext())
            {
                State g = it.next();
                if (removeAll)
                {
                    it.remove();
                    continue;
                }

                /*we assume each step is not cheaper- there it's only getting more expensive*/
                if (Board.cost(g) >= thresh) /*no point if we already passed another path's cost*/
                {
                    removeAll = true;
                    it.remove();
                    continue;
                }

                String gKey = key(g);

                /*There is already node (g') with the same i,j */
                if (H.containsKey(gKey))
                {
                    State gTag = H.get(gKey);

                    /*We finished exploring g'*/
                    if (isOut.contains(gTag))
                    {
                        it.remove();
                    }
                    else /*g' wasn't finished with*/
                    {
                        /*g' cheaper - then we remove g*/
                        if (Board.cost(gTag) <= Board.cost(g))
                        {
                            it.remove();
                        }
                        else /*g is cheaper - remove g'*/
                        {
                            L.remove(gTag);
                            H.remove(gKey);
                        }

                    }
                    continue;
                }

                if (b.isGoal(g)) /*Found goal, any branch passing this goal's cost will be cut*/
                {
                    thresh = Board.cost(g);
                    ans = g;
                    removeAll = true;
                    it.remove();
                }
                //
            }

            /* Put whatever is left from neighbor list to stack and H */
            ListIterator<State> reverser = neighbors.listIterator(neighbors.size());
            /*Put the cheapest nodes first*/
            while(reverser.hasPrevious())
            {
                State leftover = reverser.previous();
                L.push(leftover);
                H.put(key(leftover), leftover);
            }
        }

        if (ans == null) return FAIL;
        return path(ans);
    }


    public void showSolution()
    {
        StringBuilder ans = new StringBuilder();
        for (int i = 0; i < Board.N; i++)
        {
            for (int j = 0; j < Board.N; j++)
            {
                /* Cliffs */
                if (Board.myBoard[i][j].type == 'X')
                {
                    ans.append(RED);
                }

                /* Start */
                if (b.start_i == i && b.start_j == j)
                {
                    ans.append(CYAN);
                }

                /* Path to goal */
                if (solution.contains(key(i, j)))
                {
                    ans.append(GREEN);
                }

                /* Goal */
                if (Board.goal_i == i && Board.goal_j == j)
                {
                    ans.append(YELLOW);
                }

                ans.append(Board.myBoard[i][j].type);
                ans.append(RESET);
                ans.append(" ");

            }
            ans.append('\n');
        }
        System.out.println(ans);
    }


    public static final String RESET = "\u001B[0m";
    public static final String GREEN = "\033[0;92m";
    public static final String YELLOW = "\033[0;93m";
    public static final String CYAN = "\033[0;96m";
    public static final String RED = "\033[0;91m";

// END CLASS
}
