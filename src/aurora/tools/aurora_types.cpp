#include "aurora_types.h"
#include <core/typedefs.h>

namespace aurora {

Unit2::Unit2(Unit iX, Unit iY)
    : x(iX)
    , y(iY)
{
}

Unit2::Unit2(Unit xy)
    : x(xy)
    , y(xy)
{
}

Unit2 Unit2::operator+(Unit2 const&p_v) const
{
    return Unit2(x + p_v.x, y + p_v.y);
}

UnitRect::UnitRect()
    : position(0)
    , size(0)
{
}

UnitRect::UnitRect(Unit2 iPosition, Unit2 iSize)
    : position(iPosition)
    , size(iSize)
{
}

UnitRect UnitRect::Clip(UnitRect const& p_rect) const {

    UnitRect new_rect = p_rect;

    if (!Intersects(new_rect))
        return UnitRect();

    new_rect.position.x = MAX(p_rect.position.x, position.x);
    new_rect.position.y = MAX(p_rect.position.y, position.y);

    Unit2 p_rect_end = p_rect.position + p_rect.size;
    Unit2 end = position + size;

    new_rect.size.x = MIN(p_rect_end.x, end.x) - new_rect.position.x;
    new_rect.size.y = MIN(p_rect_end.y, end.y) - new_rect.position.y;

    return new_rect;
}

bool UnitRect::Intersects(UnitRect const&p_rect) const {
    if (position.x >= (p_rect.position.x + p_rect.size.x))
        return false;
    if ((position.x + size.x) <= p_rect.position.x)
        return false;
    if (position.y >= (p_rect.position.y + p_rect.size.y))
        return false;
    if ((position.y + size.y) <= p_rect.position.y)
        return false;

    return true;
}

}
