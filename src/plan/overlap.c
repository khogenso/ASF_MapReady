#include "plan.h"
#include "plan_internal.h"

OverlapInfo *overlap_new(int pct, int n, Polygon *viewable_region,
                         int zone, double clat, double clon, stateVector *st,
                         double t)
{
    OverlapInfo *oi = MALLOC(sizeof(OverlapInfo));
    oi->pct = ((double)pct)/((double)n);
    oi->viewable_region = viewable_region;
    oi->utm_zone = zone;
    oi->state_vector = *st;
    oi->clat = clat;
    oi->clon = clon;
    oi->t = t;

    return oi;
}

void overlap_free(OverlapInfo *oi)
{
    // don't free the "viewable_region" member, we do not own that
    FREE(oi);
}
