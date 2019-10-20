using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;


public class Mover : MonoBehaviour
{
    public GameObject fire;

    Thread th = null;
    TcpClient tcp = null;
    NetworkStream ns = null;

    bool bj = false;

    double p = 0, r = 0, yw = 0;
    double vr = 0, hr = 0;
    double x, y, z;
    double xa, ya, za;
    double xr, yr, zr;

    Vector3 obj;
    UnityEngine.Random rand = new UnityEngine.Random();
    
    void Start()

    {
        fire.active = false;

        x = 0;
        y = 0;
        z = 0;

        xa = 0;
        ya = 0;
        za = 0;

        xr = 0;
        yr = 0;

        if (tcp == null)
        {
            tcp = new TcpClient("192.168.4.1", 80);
            ns = tcp.GetStream();

            th = new Thread(rx_thread);
            th.Start();

        }
    }

    double DeadSpace(double data, double range)
    {
        if (Math.Abs(data) > range)
        {
            if (data > 0)
                data -= range;

            if (data < 0)
                data += range;
        }

        else
        {
            data = 0;
        }

        return data;
    }

    void OnGUI()
    {
        GUI.Label(new Rect(Screen.width - 100, 10, 100, 40), "xa:" + xa.ToString("0.##"));
        GUI.Label(new Rect(Screen.width - 100, 20, 100, 40), "ya:" + ya.ToString("0.##"));
        GUI.Label(new Rect(Screen.width - 100, 30, 100, 40), "za:" + za.ToString("0.##"));

        GUI.Label(new Rect(Screen.width - 200, 10, 100, 40), "x:" + x.ToString("0.##"));
        GUI.Label(new Rect(Screen.width - 200, 20, 100, 40), "y:" + y.ToString("0.##"));
        GUI.Label(new Rect(Screen.width - 200, 30, 100, 40), "z:" + z.ToString("0.##"));

        GUI.Label(new Rect(Screen.width - 200, 50, 100, 40), "bj:" + bj.ToString());
    }
 
    void FixedUpdate()
    {
        if (bj)
        {
            xa = 0;
            ya = 0;

            xr = -p;
            yr = r;

            DeadSpace(xr, 5);

            DeadSpace(yr, 5);

            transform.Rotate(new Vector3((float)xr / 50.0f, (float)yr / 50.0f, 0));
        }

        else
        {
            xa = r;
            ya = p;

            DeadSpace(xa, 5);
            xa /= 100.0;

            DeadSpace(ya, 5);
            ya /= 100.0;

            xa *= Math.Abs(xa);
            xa /= 0.4;

            ya *= Math.Abs(ya);
            ya /= 0.4;

        }

        zr = hr;

        DeadSpace(zr, 10);
        zr /= 50.0;

        CameraControl.angle += (float)zr;

        za = vr;

        DeadSpace(za, 2);
        za /= 100.0;

        za *= Math.Abs(za);
        za /= 0.4;

        CameraControl.vbr = (float)z / 30.0f;

        if (x < xa)
            x += 0.0008;
        else if(x > xa)
            x -= 0.0008;

        if (y < ya)
            y += 0.0008;
        else if (y > ya)
            y -= 0.0008;

        if (z < za)
            z += 0.0015;
        else if (z > za)
            z -= 0.0015;

        if(za > 0.1)
        {
            fire.active = true;
        }
        else
        {
            fire.active = false;
        }

        transform.Translate((float)x, (float)y, (float)z);
    }

    void rx_thread()
    {
        string bf = "";
        while (true)
        {
            int tmp = ns.ReadByte();
            if (tmp != -1)
            {
                bf += (char)tmp;
                if (tmp == '\n')
                {
                    int stp = 0;
                    string st = "";
                    List<string> pr = new List<string> { };
                    for (int i = 0; i < bf.Length; i++)
                    {
                        if (stp == 0 && bf[i] == '\"')
                        {
                            stp = 1;
                        }
                        else if (stp == 1 && bf[i] == '\"')
                        {
                            stp = 0;
                            pr.Add(st);
                            st = "";
                        }
                        else if (stp == 1)
                        {
                            st += bf[i];
                        }
                    }

                    if (pr.Count == 9)
                    {

                        try
                        {
                            p = double.Parse(pr[0]) / 10;
                            r = double.Parse(pr[1]) / 10;
                            yw = double.Parse(pr[2]) / 5;
                            vr = double.Parse(pr[3]) / 2;
                            hr = double.Parse(pr[4]) / 2;

                            if (pr[5] == "1")
                                bj = true;
                            else
                                bj = false;

                            if (pr[7] == "1")
                                CameraControl.cam_rev = true;
                            else
                                CameraControl.cam_rev = false;

                            p = DeadSpace(p, 10);
                            r = DeadSpace(r, 10);
                            yw = DeadSpace(yw, 10);

                            vr = DeadSpace(vr, 3);
                            vr = DeadSpace(vr, 3);
                        }
                        catch
                        {

                        }
                        if (p > 50)
                            p = 50;
                        if (p < -50)
                            p = -50;

                        if (r > 50)
                            r = 50;
                        if (r < -50)
                            r = -50;

                        if (yw > 50)
                            yw = 50;
                        if (yw < -50)
                            yw = -50;

                        if (vr > 50)
                            vr = 50;
                        if (vr < -50)
                            vr = -50;

                        if (hr > 50)
                            hr = 50;
                        if (hr < -50)
                            hr = -50;

                    }

                    bf = "";
                }
            }
        }
    }
}
