import edu.princeton.cs.introcs.StdDraw;

import javax.sound.sampled.*;
import java.awt.*;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

public class Pong
{
    pongPlayer leftPlayer;
    pongPlayer rightPlayer;
    ArrayList<pongBall> b;
    TimerTask t;

    static final double left = 0;
    static final double right = 1;
    static final double startingSpeed = 0.004;
    static final double ballModifier = 0.04;
    static final Color ballCol = Color.gray;

    static final double startingPSpeed = 0.015;
    static final double pSpeedHigh = 0.020;

    public Pong()
    {
        this.b = new ArrayList<>();
        double leftCorner = left;
        double rightCorner = right;
        double cen = (leftCorner + rightCorner) / 2;

        double leftStart = (leftCorner + rightCorner) * 0.1;
        double rightStart = (leftCorner + rightCorner) * 0.9;

        double pHeight = (leftCorner + rightCorner) * 0.4;
        double pWidth = (leftCorner + rightCorner) * 0.05;
        double pSpeed = (leftCorner + rightCorner) * 0.01;
        this.leftPlayer = new pongPlayer(leftStart, cen, pWidth, pHeight, pSpeed, KeyEvent.VK_W, KeyEvent.VK_S);
        this.rightPlayer = new pongPlayer(rightStart, cen, pWidth, pHeight, pSpeed, KeyEvent.VK_UP, KeyEvent.VK_DOWN);

        double ballSize = (leftCorner + rightCorner) * ballModifier;
        double[] ballStart = {cen, cen};

        double z = startingSpeed;
        for (int i = 0; i < 3; i++)
        {
            this.b.add(new pongBall(ballStart[0], ballStart[1], ballSize, Math.random() * z - 2 * z, this));
        }

        /*  Timer object to increase speed and colour at defined intervals.  */
        this.t = new TimerTask() {
            @Override
            public void run()
            {
                for (pongBall bb : b)
                    if (Math.abs(bb.vel.x) < startingPSpeed)
                    {
                        bb.vel.x *= 1.07;
                        int r = bb.cl.getRed() + 4;
                        int g = bb.cl.getGreen() + 4;
                        int b = bb.cl.getBlue() + 4;

                        if (r < 255 && g < 255 && b < 255)
                            bb.cl = new Color(r,g,b);
                    }
            }
        };
        long delay = 1000L;
        Timer timer = new Timer();
        timer.schedule(t, delay, delay);
        Font font = new Font("Monospaced", Font.BOLD, 40);
        StdDraw.setFont(font);
    }

    public void start()
    {
        boolean run = true;
        while (run)
        {
            clear();

            leftPlayer.update();
            rightPlayer.update();

            leftPlayer.draw();
            rightPlayer.draw();

            for (pongBall bb : this.b)
            {
                bb.update();
                bb.draw();
            }

            drawScore();
            drawBorders();

            run = checkExit();
            this.delay();
            StdDraw.show(0);
        }
        System.exit(0);
    }

    private void clear()
    {
        StdDraw.clear(Color.black);
        StdDraw.setPenColor(ballCol);
    }

    /** Returns ball to center of screen, resets speed, awards the scoring player
     * and directs the ball to the scoring player.
     *
     */
    public void reset(pongPlayer p, pongBall bb)
    {
        double cen = (right + left) / 2;
        System.out.println(bb.vel.x);

        bb.x = cen;
        bb.y = cen;
        bb.vel.x = Math.random() * startingSpeed;
        bb.vel.y = 0;
        boolean facingLoser = (cen > p.x && bb.vel.x < 0)
                || (cen < p.x && bb.vel.x > 0);

        if (facingLoser)
            bb.vel.x *= -1;

        /*  Losing player will be faster */
        if (leftPlayer.score > rightPlayer.score)
        {
            rightPlayer.speed = pSpeedHigh;
            leftPlayer.speed = startingPSpeed;
        }

        else if (rightPlayer.score > leftPlayer.score)
        {
            leftPlayer.speed = pSpeedHigh;
            rightPlayer.speed = startingPSpeed;
        }
        else
        {
            leftPlayer.speed = startingPSpeed;
            rightPlayer.speed = startingPSpeed;
        }
        bb.cl = ballCol;
    }

    private void delay()
    {
        try {
            Thread.sleep(1000 / 60);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void drawScore()
    {
        StdDraw.setPenColor(Color.orange);
        StdDraw.text(this.leftPlayer.x, right * 0.9, this.leftPlayer.score+"");
        StdDraw.text(this.rightPlayer.x, right * 0.9, this.rightPlayer.score+"");
    }

    private void drawBorders()
    {
        StdDraw.polygon(new double[]{left, right, right, left}, new double[]{left, left, right, right});
    }

    private boolean checkExit()
    {
        return !StdDraw.isKeyPressed(KeyEvent.VK_ESCAPE);
    }

    public void hitSound(String sound)
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
        Pong session = new Pong();
        session.start();
    }

}

class pongBall
{
    Velocity vel;
    double x, y, r, speed;
    pongPlayer p1, p2;
    Pong p;
    int hitCd;
    Color cl;

    public pongBall(double x, double y, double r, double speed, Pong p)
    {
        this.x = x;
        this.y = y;
        this.r = r;
        this.speed = speed;
        this.p = p;
        this.p1 = p.leftPlayer;
        this.p2 = p.rightPlayer;
        this.vel = new Velocity(speed, 0);
        cl = Pong.ballCol;
    }

    public void update()
    {
        decreaseCd();
        this.x += vel.x;
        this.y += vel.y;

        if (hit() && this.hitCd == 0)
        {
            this.hitCd = 20;
            this.p.hitSound("block_hit.wav");
            this.vel.x *= -1;
            double centerDist = this.y - this.p1.y;
            if (in(p2))
                centerDist = this.y - this.p2.y;

            this.vel.y = centerDist * 0.01;
        }

        /*  Out of bounds check  */
        if (this.x > Pong.right)
        {
            if (this.p1.x > this.p2.x)
            {
                this.p2.score();
                this.p.reset(p1, this);
            }
            else
            {
                this.p1.score();
                this.p.reset(p2, this);
            }
        }
        else if (this.x < Pong.left)
        {
            if (this.p1.x < this.p2.x)
            {
                this.p2.score();
                this.p.reset(p1, this);
            }
            else
            {
                this.p1.score();
                this.p.reset(p2, this);
            }
        }/*     Out of bounds end check     */

        /*  Roof hit check  */
        if (this.y - this.r < Pong.left || this.y + this.r> Pong.right)
            this.vel.y *= -1;
    }

    private void decreaseCd()
    {
        if (this.hitCd > 0)
            this.hitCd--;
    }

    public boolean hit()
    {
        return in(this.p1) || in(this.p2);
    }

    /** Returns true if ball is inside the player's rectangle.
     *
     */
    public boolean in(pongPlayer p)
    {
        double half = 0.5;
        double x1 = p.x - half * p.w;
        double y1= p.y - half * p.h;
        double x2 = p.x + half * p.w;
        double y2 = p.y + half * p.h;

        boolean pastX = this.x - half * p.w < x2;
        if (p.x > (Pong.right + Pong.left) / 2)
            pastX = this.x + half * p.w> x1;

        return (this.x + this.r > x1 && this.x - this.r < x2 && this.y + this.r > y1 && this.y - this.r < y2)
                && !pastX;
    }

    public void draw()
    {
        StdDraw.setPenColor(cl);
        StdDraw.filledCircle(this.x, this.y, this.r);
    }
}

class pongPlayer
{
    int upKey, downKey, score;
    double x, y, w, h, speed;
    public pongPlayer(double x, double y, double w, double h, double speed, int up, int down)
    {
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
        this.speed = speed;
        this.upKey = up;
        this.downKey = down;
        this.score = 0;
    }
    public void update()
    {
        if (StdDraw.isKeyPressed(upKey))
        {
            this.y += this.speed;
        }
        else if (StdDraw.isKeyPressed(downKey))
        {
            this.y -= this.speed;
        }
    }

    public void score()
    {
        this.score++;
    }
    public void draw()
    {
        StdDraw.filledRectangle(this.x, this.y, 0.5 * this.w, 0.5 * this.h);
    }
    public void drawEdge()
    {
        StdDraw.rectangle(this.x, this.y, 0.5 * this.w, 0.5 * this.h);
    }
}


