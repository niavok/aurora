#ifndef AURORA_TYPES_H
#define AURORA_TYPES_H

#include <stdint.h>
#include <core/math/rect2.h>

namespace aurora {

struct int2
{
	int x;
	int y;
};

typedef int64_t energy;
typedef int64_t Quantity;

typedef int32_t Unit;

struct Unit2
{
    Unit2(Unit xy);
    Unit2(Unit iX, Unit iY);

    Unit x;
    Unit y;

    Unit2 operator+(Unit2 const& p_v) const;
    Unit2 operator*(Unit v) const;
    bool operator==(Unit2 const&p_v) const { return x== p_v.x && y == p_v.y; }

    Vector2 ToVector2() const;
};



struct UnitRect
{
    UnitRect();
    UnitRect(Unit2 iPosition, Unit2 iSize);
    UnitRect(Unit x, Unit y, Unit width, Unit height);

    UnitRect Clip(UnitRect const& p_rect) const;
    bool Intersects(UnitRect const&p_rect) const;

    inline bool IsEmpty() const {
        return (size.x <= 0 || size.y <= 0);
    }

    bool operator==(UnitRect const&p_rect) const { return position== p_rect.position && size == p_rect.size; }


    Unit2 position;
    Unit2 size;
};



typedef float Scalar;

inline float sqr(Scalar x) { return x*x; }

}


#endif
