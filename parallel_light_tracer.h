#ifndef PARALLEL_LIGHT_TRACER_H
#define PARALLEL_LIGHT_TRACER_H

#include "wtelescope.h"

struct ObstructionSwitch
{
    bool door;
    bool camera;
    bool duct;
    bool duct_stair;
    bool led_frame;
    bool container_wall;

    ObstructionSwitch()
        : door(true),
          camera(true),
          duct(true),
          duct_stair(true),
          led_frame(true),
          container_wall(true)
    {
    }
};

enum TraceStatus
{
    TRACE_HIT_MIRROR         =  1,
    TRACE_BLOCKED_DOOR       = -1,
    TRACE_BLOCKED_CAMERA     = -2,
    TRACE_BLOCKED_CONTAINER  = -3,
    TRACE_MISS_MIRROR        = -5,
    TRACE_BLOCKED_DUCT       = -6,
    TRACE_BLOCKED_DUCT_STAIR = -7,
    TRACE_BLOCKED_LED        = -9,
    TRACE_INVALID_DIRECTION  = -10
};

class ParallelLightTracer
{
public:
    explicit ParallelLightTracer(WTelescope *telescope);

    int TraceToMirror(
        double x0,
        double y0,
        double z0,
        double m,
        double n,
        double l,
        double &xmirror,
        double &ymirror,
        double &zmirror,
        int &mirror_i,
        int &mirror_m,
        const ObstructionSwitch &sw
    );

    int TraceMirrorToDiffuseTarget(
        double xmirror,
        double ymirror,
        double zmirror,
        int mirror_i,
        int mirror_m,
        double m1,
        double n1,
        double l1,
        double &xtarget,
        double &ytarget,
        double &m2,
        double &n2,
        double &l2
    );

    int TraceLambertToMirror(
        double x0,
        double y0,
        double z0,
        double m,
        double n,
        double l,
        double &xmirror,
        double &ymirror,
        double &zmirror,
        int &mirror_i,
        int &mirror_m,
        const ObstructionSwitch &sw
    );

private:
    WTelescope *telescope_;
};

#endif
