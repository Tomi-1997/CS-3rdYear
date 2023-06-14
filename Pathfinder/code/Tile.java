/* A single tile that is used to build a board. */
package code;
public class Tile
{
    char type;
    int cost;
    public Tile(char t, int c)
    {
        this.type = t;
        this.cost = c;
    }
}
