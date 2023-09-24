/*****************************************************************************
 *
 *  PROJECT:        SampX
 *  LICENSE:        See LICENSE in the top level directory
 *  FILE:           SampVersions.cpp
 *  DESCRIPTION:    Determining the version of the samp by file
 *  COPYRIGHT:      (c) 2021, 2023 RINWARES <rinwares.com>
 *  AUTHOR:         Rinat Namazov <rinat.namazov@rinwares.com>
 *
 *****************************************************************************/

#include <pe-parse/parse.h>

#include "SampVersions.hpp"

SampVersion SampVersions::getVersionOfSampDll(const QString& filePath)
{
    auto filePe{peparse::ParsePEFromFile(filePath.toLocal8Bit().data())};

    switch (filePe->peHeader.nt.OptionalHeader.AddressOfEntryPoint) {
        case 0x2E2BB7:
            return SampVersion::SAMP_03E;
        case 0x31DF13:
            return SampVersion::SAMP_037_R1;
        case 0x3195DD:
            return SampVersion::SAMP_037_R2;
        case 0xCC490:
            return SampVersion::SAMP_037_R3;
        case 0xCC4D0:
            return SampVersion::SAMP_037_R3_1;
        case 0xCBCD0:
            return SampVersion::SAMP_037_R4;
        case 0xCBCB0:
            return SampVersion::SAMP_037_R4_2;
        case 0xCBC90:
            return SampVersion::SAMP_037_R5;
        case 0xFDB60:
            return SampVersion::SAMP_03DL_R1;
    }

    return SampVersion::SAMP_UNKNOWN;
}

QString SampVersions::getSampVersionAsString(SampVersion sampVersion, bool includeName)
{
    QString ver{includeName ? "SA-MP " : ""};

    switch (sampVersion) {
        case SampVersion::SAMP_03E:
            ver += "0.3e";
            break;
        case SampVersion::SAMP_037_R1:
            ver += "0.3.7 R1";
            break;
        case SampVersion::SAMP_037_R2:
            ver += "0.3.7 R2";
            break;
        case SampVersion::SAMP_037_R3:
            ver += "0.3.7 R3";
            break;
        case SampVersion::SAMP_037_R3_1:
            ver += "0.3.7 R3-1";
            break;
        case SampVersion::SAMP_037_R4:
            ver += "0.3.7 R4";
            break;
        case SampVersion::SAMP_037_R4_2:
            ver += "0.3.7 R4-2";
            break;
        case SampVersion::SAMP_037_R5:
            ver += "0.3.7 R5";
            break;
        case SampVersion::SAMP_03DL_R1:
            ver += "0.3.DL R1";
            break;
        default:
            break;
    }
    return ver;
}
