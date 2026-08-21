#include "parallel_light_tracer.h"

#include <cmath>

ParallelLightTracer::ParallelLightTracer(WTelescope *telescope)
    : telescope_(telescope)
{
}

int ParallelLightTracer::TraceToMirror(
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
)
{
    double x = 0.0;
    double y = 0.0;

    if(sw.door)
    {
        telescope_->Plane(
            ZDOOR,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x) > D_DOOR / 2.0 ||
            std::fabs(y) > Hdoor / 2.0
        )
            return TRACE_BLOCKED_DOOR;
    }

    if(sw.camera)
    {
        telescope_->Plane(
            ZCLUSTER1,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x) < CLUSTER_X / 2.0 &&
            std::fabs(y) < CLUSTER_Y / 2.0
        )
            return TRACE_BLOCKED_CAMERA;

        telescope_->Plane(
            ZCLUSTER0,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x) < CLUSTER_X / 2.0 &&
            std::fabs(y) < CLUSTER_Y / 2.0
        )
            return TRACE_BLOCKED_CAMERA;
    }

    if(sw.duct)
    {
        telescope_->Plane(
            ZCLUSTER0,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(y - Duct_Height1) < Duct_Width / 2.0 &&
            std::fabs(x) < Duct_Length / 2.0
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(y - Duct_Height2) < Duct_Width / 2.0 &&
            std::fabs(x) < Duct_Length / 2.0
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ1,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ2,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ3,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ4,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;
    }

    if(sw.duct_stair)
    {
        telescope_->Plane(
            FOCUS,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < DuctStair_Length / 2.0 &&
            y < DuctStair_Height
        )
            return TRACE_BLOCKED_DUCT_STAIR;

        if(
            std::fabs(x - DuctX2) < DuctStair_Length / 2.0 &&
            y < DuctStair_Height
        )
            return TRACE_BLOCKED_DUCT_STAIR;
    }

    if(sw.led_frame)
    {
        telescope_->Plane(
            LedFrameZ,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x) < LedFrameWidth / 2.0 &&
            y < LedFrameHeight
        )
            return TRACE_BLOCKED_LED;

        if(
            x * x +
            (y - LedFrameHeight) * (y - LedFrameHeight) <
            LedFrameRadius * LedFrameRadius
        )
            return TRACE_BLOCKED_LED;
    }

    telescope_->Sphere(
        ZMIRROR,
        CURVATURE,
        x0, y0, z0,
        m, n, l,
        &xmirror,
        &ymirror,
        &zmirror
    );

    if(sw.container_wall)
    {
        if(
            std::fabs(xmirror) > D_DOOR / 2.0 ||
            std::fabs(ymirror) > Hdoor / 2.0
        )
            return TRACE_BLOCKED_CONTAINER;
    }

    double deltax = -10000.0;
    double deltay = -10000.0;
    double deltaz = -10000.0;

    mirror_i = -1;
    mirror_m = -1;

    telescope_->WhichMirror(
        xmirror,
        ymirror,
        zmirror,
        &deltax,
        &deltay,
        &deltaz,
        &mirror_i,
        &mirror_m
    );

    if(deltax == -10000.0)
        return TRACE_MISS_MIRROR;

    return TRACE_HIT_MIRROR;
}

int ParallelLightTracer::TraceMirrorToDiffuseTarget(
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
)
{
    const double TARGET_Z = ZCLUSTER0;
    const double TARGET_WIDTH = 200.0;
    const double TARGET_HEIGHT = 200.0;

    (void)mirror_i;
    (void)mirror_m;

    double normal_x = -xmirror;
    double normal_y = -ymirror;
    double normal_z = CURVATURE - zmirror;

    double normal_length = std::sqrt(
        normal_x * normal_x +
        normal_y * normal_y +
        normal_z * normal_z
    );

    if(normal_length <= 0.0)
        return TRACE_INVALID_DIRECTION;

    normal_x /= normal_length;
    normal_y /= normal_length;
    normal_z /= normal_length;

    telescope_->GetReflected(
        m1,
        n1,
        l1,
        normal_x,
        normal_y,
        normal_z,
        &m2,
        &n2,
        &l2
    );

    telescope_->Plane(
        TARGET_Z,
        xmirror,
        ymirror,
        zmirror,
        m2,
        n2,
        l2,
        &xtarget,
        &ytarget
    );

    if(
        std::fabs(xtarget) > TARGET_WIDTH / 2.0 ||
        std::fabs(ytarget) > TARGET_HEIGHT / 2.0
    )
        return -2;

    return TRACE_HIT_MIRROR;
}

int ParallelLightTracer::TraceLambertToMirror(
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
)
{
    double x = 0.0;
    double y = 0.0;

    if(l >= 0.0)
        return TRACE_INVALID_DIRECTION;

    /*
     * Lambert light starts from the diffuse target at
     * z = ZCLUSTER0 and propagates toward Z-.
     *
     * Door and camera obstruction are intentionally not tested.
     */

    if(sw.duct)
    {
        /*
         * Horizontal ducts at the target/focal plane.
         * The 200 x 200 mm target itself does not overlap these
         * ducts, but the test is retained for geometric consistency.
         */
        if(
            std::fabs(y0 - Duct_Height1) < Duct_Width / 2.0 &&
            std::fabs(x0) < Duct_Length / 2.0
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(y0 - Duct_Height2) < Duct_Width / 2.0 &&
            std::fabs(x0) < Duct_Length / 2.0
        )
            return TRACE_BLOCKED_DUCT;

        /*
         * Side ducts at DuctZ1.
         * DuctZ1 = FOCUS = target z, therefore the starting
         * coordinates are used directly.
         */
        if(
            std::fabs(x0 - DuctX1) < Duct_Width / 2.0 &&
            y0 < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x0 - DuctX2) < Duct_Width / 2.0 &&
            y0 < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ2,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height1
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ3,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        telescope_->Plane(
            DuctZ4,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x - DuctX1) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;

        if(
            std::fabs(x - DuctX2) < Duct_Width / 2.0 &&
            y < Duct_Height2
        )
            return TRACE_BLOCKED_DUCT;
    }

    if(sw.duct_stair)
    {
        /*
         * Duct stair is also located at the focal plane.
         * The Lambert ray starts at this plane, so test x0/y0.
         */
        if(
            std::fabs(x0 - DuctX1) < DuctStair_Length / 2.0 &&
            y0 < DuctStair_Height
        )
            return TRACE_BLOCKED_DUCT_STAIR;

        if(
            std::fabs(x0 - DuctX2) < DuctStair_Length / 2.0 &&
            y0 < DuctStair_Height
        )
            return TRACE_BLOCKED_DUCT_STAIR;
    }

    if(sw.led_frame)
    {
        telescope_->Plane(
            LedFrameZ,
            x0, y0, z0,
            m, n, l,
            &x, &y
        );

        if(
            std::fabs(x) < LedFrameWidth / 2.0 &&
            y < LedFrameHeight
        )
            return TRACE_BLOCKED_LED;

        if(
            x * x +
            (y - LedFrameHeight) * (y - LedFrameHeight) <
            LedFrameRadius * LedFrameRadius
        )
            return TRACE_BLOCKED_LED;
    }

    telescope_->Sphere(
        ZMIRROR,
        CURVATURE,
        x0, y0, z0,
        m, n, l,
        &xmirror,
        &ymirror,
        &zmirror
    );

    if(sw.container_wall)
    {
        if(
            std::fabs(xmirror) > D_DOOR / 2.0 ||
            std::fabs(ymirror) > Hdoor / 2.0
        )
            return TRACE_BLOCKED_CONTAINER;
    }

    double deltax = -10000.0;
    double deltay = -10000.0;
    double deltaz = -10000.0;

    mirror_i = -1;
    mirror_m = -1;

    telescope_->WhichMirror(
        xmirror,
        ymirror,
        zmirror,
        &deltax,
        &deltay,
        &deltaz,
        &mirror_i,
        &mirror_m
    );

    if(deltax == -10000.0)
        return TRACE_MISS_MIRROR;

    return TRACE_HIT_MIRROR;
}

