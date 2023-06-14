/* Executes a given pathfinding algorithm on a given input text file. */
package code;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Scanner;

public class Ex1
{
    public static void main(String[] args)
    {
        /* Read from file input */
        String whichAlgo, clockDir, timeOption,
                listPrintOption, boardSize, startEndPos;
        int N;
        String[] boardInput;
        boolean prefOld = true;

        try
        {
            String userDirectory = System.getProperty("user.dir");
            String path = userDirectory + "/src/tests/hTest1.txt";
            if (args.length == 1)
            {
                path = args[0];
            }
            File myObj = new File(path);
            Scanner myReader = new Scanner(myObj);

            whichAlgo = myReader.nextLine();
            if (whichAlgo == null) return;

            clockDir = myReader.nextLine();
            if (clockDir == null) return;

            String algo = whichAlgo.toLowerCase();

            /* For A* or DFBnb read new-first or old-first */
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

            /* File -> code.Board object */
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

            /* For debug - showing failure type through the server's response */
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

        /* Input to algo and board objects */
        Board b = new Board();
        b.initBoard(boardInput);

        /* Init operators based on direction */
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

        /* Run algorithm */

        Pathfinder p = new Pathfinder(whichAlgo, clockDir, timeOption, listPrintOption, b);

        /* When two states with the same cost are compared, the next comparison is:
         * (this.age - other.age) * AGE_PREF
         * Then if prefOld is true, older states will be preferred.*/
        Pathfinder.AGE_PREF = prefOld? 1 : -1;

        p.run();

        System.out.println(b);
        p.showSolution();

    }
}

