/* Runs a defined list of inputs and prints where the mistake was */
package code;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Scanner;

public class Test
{

    public static void main(String[] args)
    {
        // Template        name     ---      cost ---  nodes created
        // nameCost.put("test.txt", new Integer[]{0, 0});

        HashMap<String, Integer[]> nameCost = new HashMap<>();
        nameCost.put("bfs.txt", new Integer[]{21, 28});  // counter-clockwise makes the path stick to left

        nameCost.put("dfid.txt", new Integer[]{45, 37}); // the upper branch has a smaller depth than the lower branch

        nameCost.put("a_s.txt", new Integer[]{21, 14});  // two branches are created to the end, left is cheaper

        nameCost.put("agePref.txt", new Integer[]{6, 5});  // Looking at output and making sure A* picks the relevant path.

        nameCost.put("ida_s.txt", new Integer[]{15, 6}); // will create path to goal three times

        nameCost.put("dfbnb.txt", new Integer[]{13, 12});// will create all the board even after creating goal
                                                        // as the goal is slightly more expensive than the last node

        nameCost.put("hTest1.txt", new Integer[]{52, -1}); // Testing optimality, counting nodes manually is a headache
        nameCost.put("hTest2.txt", new Integer[]{52, -1}); // Testing optimality, counting nodes manually is a headache
        nameCost.put("hTest3.txt", new Integer[]{52, -1}); // Testing optimality, counting nodes manually is a headache

        int i = 1;
        for (String filename : nameCost.keySet())
        {
            int expectedCost = nameCost.get(filename)[0];
            int expectedNumCreated = nameCost.get(filename)[1];

            System.out.print(i + ") ");
            testInput(filename, expectedCost, expectedNumCreated);
            i++;
        }
    }


    public static void testInput(String filename, int expectedCost, int expectedNum)
    {
        String whichAlgo, clockDir, timeOption,
                listPrintOption, boardSize, startEndPos;
        int N;
        String[] boardInput;
        boolean prefOld = true;

        try
        {
            String userDirectory = System.getProperty("user.dir");
            String path = userDirectory + "/src/tests/" + filename;
            File myObj = new File(path);
            Scanner myReader = new Scanner(myObj);

            whichAlgo = myReader.nextLine();
            if (whichAlgo == null) return;

            clockDir = myReader.nextLine();
            if (clockDir == null) return;

            String algo = whichAlgo.toLowerCase();
            if (algo.compareTo("a*") == 0 || algo.compareTo("dfbnb") == 0)
            {
                String prefer = clockDir.split(" ")[1];
                clockDir = clockDir.substring(0, clockDir.indexOf(" "));

                if (prefer.compareTo("new-first") == 0)
                    prefOld = false;
            }

            timeOption = myReader.nextLine();
            if (timeOption == null) return;

            listPrintOption = myReader.nextLine();
            if (listPrintOption == null) return;

            boardSize = myReader.nextLine();
            if (boardSize == null) return;

            startEndPos = myReader.nextLine();
            if (startEndPos == null) return;

            N = Integer.parseInt(boardSize);
            boardInput = new String[N];

            int i = 0;
            while (myReader.hasNextLine())
            {
                String data = myReader.nextLine();
                boardInput[i] = data;
                i++;
            }
            myReader.close();
        }
        catch (FileNotFoundException e)
        {
            System.out.println("An error occurred.");
            e.printStackTrace();

            /* Debug - showing failure type through the server */
            try
            {
                FileWriter myWriter = new FileWriter("output.txt");
                myWriter.write("Couldn't open input");
                myWriter.close();
            }
            catch (IOException e2)
            {
                System.out.println("An error occurred.");
                e2.printStackTrace();
            }
            return;
        }

        Board b = new Board();
        b.initBoard(boardInput);
        b.initOps(clockDir);

        startEndPos = startEndPos.replaceAll("[^0-9,]", "");

        int[] pos = new int[4];
        String[] pos_str = startEndPos.split(",");

        for (int i = 0; i < pos.length; i++)
            pos[i] = Integer.parseInt(pos_str[i]);

        b.start_i = pos[0] - 1;
        b.start_j = pos[1] - 1;
        Board.goal_i = pos[2] - 1;
        Board.goal_j = pos[3] - 1;

        Pathfinder p = new Pathfinder(whichAlgo, clockDir, timeOption, listPrintOption, b);
        /* When two states with the same cost are compared, the next comparison is:
         * (this.age - other.age) * AGE_PREF
         * Then if prefOld is true, older states will be preferred.*/
        Pathfinder.AGE_PREF = prefOld? 1 : -1;

        p.run();
        boolean success = true;

        if (expectedCost != -1)
            success = success && (expectedCost == p.finalCost);

        if (expectedNum != -1)
            success = success && (p.nodesCreated == expectedNum);

        String result = success? "passed." : "failed.";
        if (!success)
        {
            System.err.println(filename + ", actual cost - " + p.finalCost + ", actual creation - " + p.nodesCreated);
        }

        System.out.println("Test results - " + result);
    }
}
