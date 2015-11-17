#pragma once

#ifndef BOOL
#define BOOL    int
#endif
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

typedef struct {
    double Accel_X;
    double Accel_Y;
    double Accel_Z;
}AccelData;

class CAngleSensor
{
private:
    struct RawData
    {
        AccelData AccelData_Dock;
        AccelData AccelData_Lid;
    }m_RawData;

public:
    BOOL SetRawData_Lid(double X, double Y, double Z);
    BOOL SetRawData_Dock(double X, double Y, double Z);
    BOOL GetAngleDegrees(double &fAngle);

    /*void Test();*/
};
