/* Executes Kohonen algorithm to cluster points together.
 * Uses the Point_ class from the Perceptron file */

import edu.princeton.cs.introcs.StdDraw;
import java.awt.*;

public class Kohonen
{

    public static void main(String[] args)
    {
        KohonenNeuron[][] kNeurons = initNeurons();
        Point_[] clusters = generateCircle();
//        Point_[] clusters = generateXSquared();
//        Point_[] clusters = generateCluster();

        initStdDraw();
        initDrawerThread(kNeurons, clusters);

        double lr = 0.25;
        int maxIterations = 2000;
        kohonen_(clusters, kNeurons, lr, maxIterations);

    }


    // Algorithm cons
    static final int neuronsNum = 7; // Will be squared
    static final int pointsNum = 1000;
    static int neuronsUpdateRadius = 2; // Update neurons nearby to current neurons X units to all directions, decreases gradually
    static boolean converged = false;
    static final double eps = 0.0001; // Max weight update less than this- flips converged to True and exits
    static final int msDelay = 50;    // Algorithm delay between each iteration


    // StdDraw cons
    static final long fps = 1000 / 144;
    static final double nRad = 0.1;
    static final double pRad = 0.015;


    private static void initDrawerThread(KohonenNeuron[][] kNeurons, Point_[] clusters)
    {
        new Thread(() ->
        {
            while (true)
            {
                StdDraw.clear(Color.black);
                StdDraw.setPenColor(Color.darkGray);
                drawNeurons(kNeurons);

                if (!converged)
                    drawClusters(clusters);
                StdDraw.show(0);
                delay(fps);
            }
        }).start();
    }

    private static KohonenNeuron[][] initNeurons()
    {
        KohonenNeuron[][] kNeurons = new KohonenNeuron[neuronsNum][neuronsNum];
        for (int i = 0; i < neuronsNum; i++)
        {
            for (int j = 0; j < neuronsNum; j++)
            {
                kNeurons[i][j] = new KohonenNeuron(i, j);
            }
        }
        return kNeurons;
    }

    private static Point_[] generateXSquared()
    {
        Point_[] ans = new Point_[pointsNum];
        for (int i = 0; i < pointsNum; i++)
        {
            double x, y;
            do
            {
                x = Math.random() * (neuronsNum - 1) - 2;
                y = Math.random() * neuronsNum - 1;
            }
            while(y > x * x);
            ans[i] = new Point_(x+2, y, Color.lightGray);
        }
        return ans;
    }

    private static Point_[] generateCircle()
    {
        double x = (neuronsNum - 1) * 0.5;
        double y = (neuronsNum - 1) * 0.5;
        double rad = neuronsNum * 0.5;
        Point_[] ans = new Point_[pointsNum];
        for (int i = 0; i < pointsNum; i++)
        {
            double z1 = 0;
            double z2 = 0;

            do
            {
                z1 = Math.random() * 4 * rad - 2 * rad;
                z2 = Math.random() * 4 * rad - 2 * rad;
            }
            while(z1 * z1 + z2 * z2 > 2 * rad || (z1 * z1 + z2 * z2 < rad));
            ans[i] = new Point_(z1 + x, z2 + y, Color.lightGray);
        }
        return ans;
    }

    private static void kohonen_(Point_[] clusters, KohonenNeuron[][] kNeurons, double lr, int iterationsLim)
    {
        /**
         * Starts the kohonen algorithm.
         * Picks a random point X, and the closest neuron N.
         * Moves N and neighbours of N, amount of neighbours is varied, closer to X.
         * Decreases the learning rate.
         * Stops once the biggest change on one of the neurons is small.
         */
        double alpha = lr;
        delay(1000);
        for (int i = 0; i < iterationsLim; i++)
        {
            double maxChange = kohonen(clusters, kNeurons, alpha);
            alpha = Math.max(0, alpha - (1.0 / iterationsLim));
            delay(msDelay);

            if (i % (iterationsLim / 3) == 0)
                neuronsUpdateRadius--;


            if (maxChange < eps && i > iterationsLim / 10)
            {
                delay(1000);
                converged = true;
                return;
            }
        }
    }

    private static double kohonen(Point_[] clusters, KohonenNeuron[][] kNeurons, double lr)
    {
        double maxChange = -1;
        int pi = (int) (Math.random() * pointsNum);
        ijPair n = closestNeurons(kNeurons, clusters[pi]);

        // Update neighbours of N
        for (int nur = 0; nur <= neuronsUpdateRadius; nur++)
        {
            for (int i = n.i - nur; i <= n.i + nur; i++)
            {
                for (int j = n.j - nur; j <= n.j + nur; j++)
                {
                    if ((i == n.i && j == n.j) || illegal(i, j)) continue;

                    double deltaI = lr * (clusters[pi].x1 - kNeurons[i][j].i);
                    kNeurons[i][j].i += deltaI;

                    double deltaJ = lr * (clusters[pi].x2 - kNeurons[i][j].j);
                    kNeurons[i][j].j += deltaJ;

                    if (Math.abs(deltaJ) > maxChange)
                        maxChange = Math.abs(deltaJ);

                    if (Math.abs(deltaI) > maxChange)
                        maxChange = Math.abs(deltaJ);
                }
            }
        }

        return maxChange;
    }

    private static boolean illegal(int i, int j)
    {
        //
        return i < 0 || j < 0 || i >= neuronsNum || j >= neuronsNum;
    }

    private static ijPair closestNeurons(KohonenNeuron[][] kNeurons, Point_ p)
    {
        ijPair res = new ijPair(0, 0);
        double minDist = dist(p, kNeurons[res.i][res.j]);

        for (int i = 0; i < neuronsNum; i++)
        {
            for (int j = 0; j < neuronsNum; j++)
            {
                double dt = dist(p, kNeurons[i][j]);
                if (dt < minDist)
                {
                    minDist = dt;
                    res.i = i;
                    res.j = j;
                }
            }
        }
        return res;
    }

    private static double dist(Point_ p, KohonenNeuron res)
    {
        double ans = (p.x1 - res.i) * (p.x1 - res.i);
        ans += (p.x2 - res.j) * (p.x2 - res.j);
        return Math.sqrt(ans);
    }

    private static Point_[] generateCluster()
    {
        Point_[] ans = new Point_[pointsNum];
        int generated = 0;

        while (generated < pointsNum)
        {
            // Generate a random points
            double x = Math.random() * (neuronsNum + 1) - 1;
            double y = Math.random() * (neuronsNum + 1) - 1;

            // Generate a batch of points around it
            int added = (int) (Math.random() * pointsNum / 20);

            // Need 200, have 180, generated 40- make it 20
            if (generated + added >= pointsNum)
                added = pointsNum - generated;

            generated += added;
            double radAround = Math.random() * 0.5;
            for (int i = 0 ; i < added; i++)
            {
                double z1 = Math.random() * radAround * 2 - radAround;
                double z2 = Math.random() * radAround * 2 - radAround;
                ans[i + generated - added] = new Point_(x + z1, y + z2, Color.lightGray);
            }
        }

        return ans;
    }

    private static void drawClusters(Point_[] clusters)
    {
        StdDraw.setPenColor(clusters[0].label);
        for (Point_ p : clusters)
        {
            StdDraw.point(p.x1, p.x2);
        }
    }

    private static void drawNeurons(KohonenNeuron[][] kn)
    {
        for (int i = 0; i < neuronsNum; i++)
        {
            for (int j = 0; j < neuronsNum; j++)
            {
                if (!converged)
                    StdDraw.filledCircle(kn[i][j].i, kn[i][j].j, nRad);

                line(i, j, i + 1, j, kn);
                line(i, j, i - 1 , j, kn);
                line(i, j, i, j + 1, kn);
                line(i, j, i, j - 1, kn);
            }
        }
    }

    private static void line(int i1, int j1, int i2, int j2, KohonenNeuron[][] kn)
    {
        if (illegal(i2, j2)) return;
        StdDraw.line(kn[i1][j1].i, kn[i1][j1].j, kn[i2][j2].i, kn[i2][j2].j);
    }


    private static void initStdDraw()
    {
        StdDraw.setPenRadius(0.003);
        StdDraw.setScale(-1.5, neuronsNum + 0.5);
    }


    private static void delay(long ms)
    {
        //
        try {Thread.sleep(ms);} catch (InterruptedException e) {e.printStackTrace();}
    }
}

class KohonenNeuron
{
    double i, j;
    public KohonenNeuron(double i, double j)
    {
        this.i = i;
        this.j = j;
    }
}

class ijPair
{
    int i, j;
    public ijPair(int i, int j)
    {
        this.i = i;
        this.j = j;
    }
}
