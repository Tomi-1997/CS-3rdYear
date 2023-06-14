/* Generates a random board, and executes one of the defined algorithms on it */
package code;
public class Randomizer
{
     static final String[] algorithms = new String[]{"bfs", "dfid", "a*", "ida*", "dfbnb"};
    public static void main(String[] args)
    {
        /* Generate settings */
        String algo, clockDir, timeOption, listPrintOption;
        String[] boardInput;

        /* code.Board size, if changed to a high value, expect all algorithms except A* to perform considerably slower */
        int N = (int) (Math.random() * 5) + 5;

        /* Generate start and goal positions */
        int sx, sy, gx, gy;
        do
        {
            sx = (int) (Math.random() * (N));
            sy = (int) (Math.random() * (N));
            gx = (int) (Math.random() * (N));
            gy = (int) (Math.random() * (N));
        }
        while ( (sx == gx && sy == gy)); // Make sure they are different

        algo = algorithms[(int)(Math.random() * (algorithms.length))];
        clockDir = Math.random() > 0.5 ? "clockwise new-first" : "counter-clockwise new-first";
        timeOption = "with time";
        listPrintOption = "no open";
        boardInput = new String[N];
        for (int i = 0; i < N; i++)
            boardInput[i] = "";

        /* Generate code.Board */
        Board b = new Board();
        char[] tiles = new char[]{'X', 'D', 'R', 'H'};
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                char type = tiles[(int)(Math.random() * (tiles.length))];

                if (i == sx && j == sy)
                    type = 'S';

                if (i == gx && j == gy)
                    type = 'G';

                boardInput[i] += type;
            }
        }

        b.initOps(clockDir);
        b.initBoard(boardInput);

        b.start_i = sx;
        b.start_j = sy;
        Board.goal_i = gx;
        Board.goal_j = gy;

        /* Run algorithm */
        Pathfinder p = new Pathfinder(algo, clockDir, timeOption, listPrintOption, b);
        Pathfinder.AGE_PREF = 1;
        p.run();

        System.out.println(b);
        System.out.println("Executing " + algo);
        System.out.println(p.nodesCreated + " states created.");
        p.showSolution();
    }
}
