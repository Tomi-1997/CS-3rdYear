/* Given an input file with a board and settings, executes a pathfinding algorithm on it. */

import code.Ex1;
import code.Randomizer;

public class Main
{
    public static void main(String[] args)
    {
        /* Run with a given input */
        String path = "dir\\some_input.txt";
        String[] arg = new String[]{path};
        Ex1.main(arg);


        /* Or run a random input */
        Randomizer.main(null);
    }
}
