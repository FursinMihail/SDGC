using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraControl : MonoBehaviour
{

    public GameObject drone;
    private Vector3 offset;

    public static float vbr = 0;

    float z, x;
    public static float angle;
    public static bool cam_rev = false;
    
    void Start()
    {
        angle = 180;

        offset = transform.position - drone.transform.position;
        transform.parent = drone.transform;
    }

    float getOffsetSize()
    {
        double size;
        size = Math.Sqrt(Math.Pow(offset.x, 2) + Math.Pow(offset.y, 2) + Math.Pow(offset.z, 2));
        return (float)(size);
    }

    void LateUpdate()
    {
        if (cam_rev)
        {
            transform.localPosition = -offset + new Vector3(UnityEngine.Random.Range(vbr, -vbr), UnityEngine.Random.Range(vbr, -vbr), 0);
            transform.LookAt(drone.transform);
        }
        else
        {
            transform.localPosition = offset + new Vector3(UnityEngine.Random.Range(vbr, -vbr), UnityEngine.Random.Range(vbr, -vbr), 0);
            transform.rotation = new Quaternion(drone.transform.rotation.x * 1.25f, drone.transform.rotation.y * 1.25f, drone.transform.rotation.z * 1.25f, drone.transform.rotation.w);
        }

    }
}
