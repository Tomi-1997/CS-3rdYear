import edu.princeton.cs.introcs.StdDraw;

import javax.sound.sampled.*;
import java.awt.*;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;

public class FireWork extends WorldObject
{
    public static final double G = 0.0002;
    public static final Color CL = Color.white;
    public static final Color[] CLs = {Color.MAGENTA, Color.cyan, Color.RED, Color.pink, Color.orange, Color.green,
                                        Color.yellow};
    Flare f;
    ArrayList<Shrapnel> sharp_arr;
    boolean isFlare = true;

    FireWork(double end_x, double end_y)
    {
        f = new Flare(end_x, end_y); /* at launch */
        sharp_arr = new ArrayList<>(); /* explodes at peak height */
    }

    @Override
    public void draw()
    {
        if (isFlare)
            f.draw();
        else
        {
            for (Shrapnel s : sharp_arr)
                s.draw();
        }
    }

    @Override
    public void update()
    {
        if (isFlare)
        {
            f.update();
            if (f.v.y <= -0.002)
                explode();
        }
        else
        {
            for (Shrapnel s : sharp_arr)
            {
                s.update();
            }
        }

        sharp_arr.removeIf(x -> x.getLife_expect() == 0);
    }

    /**
     * Spawns a large amount of points around the current flare.
     */
    public void explode()
    {
        sound("explode.wav");
        isFlare = false;
        int n = (int) (Math.random()*300) + 60;
        Color cl = CLs[(int) (CLs.length * Math.random())];
        for (int i = 0; i < n; i++)
        {
            double[] pos = getRandXY();
            Velocity v = new Velocity(pos[0], pos[1]);
            int life_expect = 200;
            sharp_arr.add(new Shrapnel(v, cl, life_expect, this.f));
        }
    }

    /**
     * Returns a vector array bound by x squared and y squared (To make the shrapnel array form a circle).
     */
    private double[] getRandXY()
    {
        double eps = 0.001;
        double[] ans = new double[2];
        double x, y;

        /* assert it is a circle function */
        do
        {
            x = Math.random() * 2 * eps - eps;
            y = Math.random() * 2 * eps - eps;
        } while (x*x + y*y > eps * eps);

        ans[0] = x;
        ans[1] = y;
        return ans;
    }

    public static void clear()
    {
        StdDraw.clear(Color.black);
        StdDraw.setPenColor(CL);
    }

    public void sound(String sound)
    {
        // Thank you https://stackoverflow.com/questions/26305/how-can-i-play-sound-in-java
        try {
            // Open an audio input stream.
            URL url = this.getClass().getClassLoader().getResource(sound);
            assert url != null;
            AudioInputStream audioIn = AudioSystem.getAudioInputStream(url);
            // Get a sound clip resource.
            Clip clip = AudioSystem.getClip();
            // Open audio clip and load samples from the audio input stream.
            clip.open(audioIn);
            clip.start();
        } catch (UnsupportedAudioFileException | IOException | LineUnavailableException e) {
            e.printStackTrace();
        }

    }

    public static void main(String[] args)
    {
        StdDraw.setScale(0, 1);
        boolean run = true;
        ArrayList<FireWork> fw = new ArrayList<>();
        int firework_cd = 0;

        while (run)
        {
            clear();
            if (firework_cd > 0)
                firework_cd--;

            if (StdDraw.mousePressed() && firework_cd == 0)
            {
               fw.add(new FireWork(StdDraw.mouseX(), StdDraw.mouseY()));
               firework_cd = 10;
            }

            for (FireWork f : fw)
            {
                f.update();
                f.draw();
            }

            StdDraw.show(0);
            try {
                Thread.sleep(1000/60);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            if (StdDraw.isKeyPressed(KeyEvent.VK_ESCAPE))
                run = false;
        }
        System.exit(0);
    }

}

class Flare extends WorldObject
{
    double end_x, end_y;
    public Flare(double ex, double ey)
    {
        setX(Math.random());
        setY(0);
        end_x = ex;
        end_y = ey;
        double diff = Math.abs(this.x - end_x) * 0.01;
        double vel_x = this.x > ex ? diff * - 1 : diff;

        double vy = (Math.random() * 0.004) - 0.002;
        v = new Velocity(vel_x, 0.016 + vy);
    }

    public void draw()
    {
        StdDraw.setPenColor(FireWork.CL);
        StdDraw.circle(x, y, 0.002);
    }

    public void update()
    {
        setX(this.x + this.v.x);
        setY(this.y + this.v.y);

        v.setY(v.getY() - FireWork.G);
    }
}

class Shrapnel extends WorldObject
{
    private int life_expect;
    private final Color cl;

    public Shrapnel(Velocity v, Color cl, int le, Flare f)
    {
        this.x = f.x;
        this.y = f.y;
        this.v = v;
        this.cl = cl;
        this.life_expect = le + (int)((Math.random() * le * 0.6) - le * 0.3);
    }
    @Override
    public void draw()
    {
        StdDraw.setPenColor(cl);
        StdDraw.point(this.x, this.y);
    }
    @Override
    public void update()
    {
        setX(this.x + this.v.x);
        setY(this.y + this.v.y);

        if (this.life_expect < 100)
        {
            double z = 0.995;
            v.setY(v.getY() * z);
            v.setX(v.getX() * z);
        }

        if (this.life_expect > 0)
            this.life_expect--;
    }

    public int getLife_expect()
    {
        return this.life_expect;
    }
}

class WorldObject
{
    Velocity v;
    double x, y;
    public void draw()
    {
        StdDraw.point(x, y);
    }
    public void update()
    {
        setX(this.x + this.v.x);
        setY(this.y + this.v.y);

        v.setY(v.getY() - FireWork.G);
    }

    public Velocity getV() {
        return v;
    }

    public void setV(Velocity v) {
        this.v = v;
    }

    public double getX() {
        return x;
    }

    public void setX(double x) {
        this.x = x;
    }

    public double getY() {
        return y;
    }

    public void setY(double y) {
        this.y = y;
    }
}
