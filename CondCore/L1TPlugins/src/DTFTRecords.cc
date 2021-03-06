#include "CondCore/ESSources/interface/registration_macros.h"
#include "CondFormats/L1TObjects/interface/L1MuDTEtaPatternLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTExtLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTPhiLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTPtaLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTQualPatternLut.h"
#include "CondFormats/L1TObjects/interface/L1MuDTTFParameters.h"
#include "CondFormats/L1TObjects/interface/L1MuDTTFMasks.h"
#include "CondFormats/DataRecord/interface/L1MuDTEtaPatternLutRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTExtLutRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTPhiLutRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTPtaLutRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTQualPatternLutRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTTFParametersRcd.h"
#include "CondFormats/DataRecord/interface/L1MuDTTFMasksRcd.h"

REGISTER_PLUGIN(L1MuDTEtaPatternLutRcd, L1MuDTEtaPatternLut);
REGISTER_PLUGIN(L1MuDTExtLutRcd, L1MuDTExtLut);
REGISTER_PLUGIN(L1MuDTPhiLutRcd, L1MuDTPhiLut);
REGISTER_PLUGIN(L1MuDTPtaLutRcd, L1MuDTPtaLut);
REGISTER_PLUGIN(L1MuDTQualPatternLutRcd, L1MuDTQualPatternLut);
REGISTER_PLUGIN(L1MuDTTFParametersRcd, L1MuDTTFParameters);
REGISTER_PLUGIN(L1MuDTTFMasksRcd, L1MuDTTFMasks);
