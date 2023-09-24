/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampVersions.hpp
 *  DESCRIPTION:    Determining the version of the samp by file
 *  COPYRIGHT:      (c) 2021, 2023 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#pragma once

#include <QString>

enum class SampVersion {
    SAMP_UNKNOWN,
    SAMP_03E,
    SAMP_037_R1,
    SAMP_037_R2,
    SAMP_037_R3,
    SAMP_037_R3_1,
    SAMP_037_R4,
    SAMP_037_R4_2,
    SAMP_037_R5,
    SAMP_03DL_R1
};

class SampVersions {
public:
    static SampVersion getVersionOfSampDll(const QString& filePath);
    static QString     getSampVersionAsString(SampVersion sampVersion, bool includeName = true);
};
