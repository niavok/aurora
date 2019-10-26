#ifndef AURORA_PHYSIC_CONSTANTS_H
#define AURORA_PHYSIC_CONSTANTS_H

#include "aurora_physic_types.h"

namespace aurora {

class PhysicalConstants
{
public:
    static constexpr Scalar gravity = 50; // m.s-2, Acceleration of the gravity
    static constexpr Scalar gasConstant = 1e-5; // Pressure . volume . K-1, from PV = NRT
    static constexpr Scalar kineticCoef = 0.001; // acceleration ratio of a fluid due to pressure difference

    static Quantity EstimateSolidNByVolume(Material material, Volume volume);

    static Volume GetSolidVolumeByN(Material material, Quantity N);

    static Quantity EstimateLiquidNByVolume(Material material, Volume volume, Scalar pressure, Scalar temperature);

    static Volume GetLiquidVolumeByN(Material material, Quantity N, Scalar pressure, Scalar temperature);

    static Energy EstimateThermalEnergy(Material material, Quantity N, Scalar temperature);

    static Quantity EstimateGasN(Volume volume, Scalar pressure, Scalar temperature);

    static Material GetDissolvedMaterial(Material material);

    static Scalar GetSpecificHeat(Material material);

    static Scalar GetMassByN(Material material);
};

}

#endif
