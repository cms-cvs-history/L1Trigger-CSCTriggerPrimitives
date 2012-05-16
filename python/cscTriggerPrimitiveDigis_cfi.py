import FWCore.ParameterSet.Config as cms

from L1Trigger.CSCCommonTrigger.CSCCommonTrigger_cfi import *
# Default parameters for CSCTriggerPrimitives generator
# =====================================================
cscTriggerPrimitiveDigis = cms.EDProducer("CSCTriggerPrimitivesProducer",
    CSCCommonTrigger,

    # if False, parameters will be read in from DB using EventSetup mechanism
    # else will use parameters from this config
    debugParameters = cms.untracked.bool(True),
    
    # Name of digi producer module(s)
    CSCComparatorDigiProducer = cms.InputTag("simMuonCSCDigis","MuonCSCComparatorDigi"),
    CSCWireDigiProducer = cms.InputTag("simMuonCSCDigis","MuonCSCWireDigi"),

    # for SLHC studies we don't want bad chambers so far
    skipbadchamber = cms.untracked.bool(True),

    # Parameters common for all boards
    commonParam = cms.PSet(
        isTMB07 = cms.bool(True),
        isMTCC = cms.bool(False),
        
        # flag for SLHC studies
        # (if true, isTMB07 should be true as well)
        isSLHC = cms.untracked.bool(False),

        # ME1a configuration:
        # naiveME1aME1b=f, smartME1aME1b=f
        #   default logic for current HW
        # naiveME1aME1b=t, smartME1aME1b=f
        #   naive: both 1/a and 1/b get an ALCT and a CLCT finders
        #   and act as separate chambers
        # naiveME1aME1b=f, smartME1aME1b=t, gangedME1a=f
        #   realistic upgrade scenario: one ALCT finder and two CLCT finders
        #   per ME11, with additional logic for A/CLCT matching
        #   with ME1a unganged
        # naiveME1aME1b=f, smartME1aME1b=t, gangedME1a=t
        #   previous case with ME1a still ganged
        # Note: gangedME1a has effect only if smartME1aME1b=t
        naiveME1aME1b = cms.untracked.bool(False),
        smartME1aME1b = cms.untracked.bool(False),
        disableME1a = cms.untracked.bool(False),
        gangedME1a = cms.untracked.bool(False),
        disableME42 = cms.untracked.bool(False)
    ),

    # Parameters for ALCT processors: old MC studies
    alctParamOldMC = cms.PSet(
        alctFifoTbins   = cms.uint32(16),
        alctFifoPretrig = cms.uint32(10),
        alctDriftDelay  = cms.uint32(3),
        alctNplanesHitPretrig = cms.uint32(2),
        alctNplanesHitPattern = cms.uint32(4),
        alctNplanesHitAccelPretrig = cms.uint32(2),
        alctNplanesHitAccelPattern = cms.uint32(4),
        alctTrigMode       = cms.uint32(3),
        alctAccelMode      = cms.uint32(1),
        alctL1aWindowWidth = cms.uint32(5),
        verbosity = cms.untracked.int32(0)
    ),

    # Parameters for ALCT processors: MTCC-II
    alctParamMTCC = cms.PSet(
        alctFifoTbins   = cms.uint32(16),
        alctFifoPretrig = cms.uint32(10),
        alctDriftDelay  = cms.uint32(3),
        alctNplanesHitPretrig = cms.uint32(2),
        alctNplanesHitPattern = cms.uint32(4),
        alctNplanesHitAccelPretrig = cms.uint32(2),
        alctNplanesHitAccelPattern = cms.uint32(4),
        alctTrigMode       = cms.uint32(2),
        alctAccelMode      = cms.uint32(0),
        alctL1aWindowWidth = cms.uint32(3),
        verbosity = cms.untracked.int32(0)
    ),

    # Parameters for ALCT processors: 2007 and later
    alctParam07 = cms.PSet(
        alctFifoTbins   = cms.uint32(16),
        alctFifoPretrig = cms.uint32(10),
        alctDriftDelay  = cms.uint32(2),
        alctNplanesHitPretrig = cms.uint32(2),
        alctNplanesHitPattern = cms.uint32(4),
        alctNplanesHitAccelPretrig = cms.uint32(2),
        alctNplanesHitAccelPattern = cms.uint32(4),
        alctTrigMode       = cms.uint32(2),
        alctAccelMode      = cms.uint32(0),
        alctL1aWindowWidth = cms.uint32(7),
        alctEarlyTbins = cms.untracked.int32(4),
        verbosity = cms.untracked.int32(0),

        alctHitPersist  = cms.untracked.uint32(6),
        alctGhostCancellationBxDepth = cms.untracked.uint32(4),
        alctGhostCancellationSideQuality = cms.untracked.bool(False),
        alctPretrigDeadtime = cms.untracked.uint32(4),
        alctUseCorrectedBx = cms.untracked.bool(False),
        alctBxOffsets = cms.untracked.vdouble(
          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
        alctNarrowMaskForR1 = cms.untracked.bool(False)
    ),

    # Parameters for ALCT processors: SLHC studies
    alctSLHC = cms.PSet(
        alctFifoTbins   = cms.uint32(16),
        alctFifoPretrig = cms.uint32(10),
        alctDriftDelay  = cms.uint32(2),
        alctNplanesHitPretrig = cms.uint32(2),
        alctNplanesHitPattern = cms.uint32(4),
        alctNplanesHitAccelPretrig = cms.uint32(2),
        alctNplanesHitAccelPattern = cms.uint32(4),
        alctTrigMode       = cms.uint32(2),
        alctAccelMode      = cms.uint32(0),
        alctL1aWindowWidth = cms.uint32(3),
        alctEarlyTbins = cms.untracked.int32(4),
        verbosity = cms.untracked.int32(0),

        alctHitPersist  = cms.untracked.uint32(6),
        alctGhostCancellationBxDepth = cms.untracked.uint32(4),
        alctGhostCancellationSideQuality = cms.untracked.bool(False),
        alctPretrigDeadtime = cms.untracked.uint32(4),
        alctUseCorrectedBx = cms.untracked.bool(False),
        alctBxOffsets = cms.untracked.vdouble(
          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
        alctNarrowMaskForR1 = cms.untracked.bool(False)
    ),

    # Parameters for CLCT processors: old MC studies
    clctParamOldMC = cms.PSet(
        clctFifoTbins   = cms.uint32(12),
        clctFifoPretrig = cms.uint32(7),
        clctHitPersist  = cms.uint32(6),
        clctDriftDelay  = cms.uint32(2),
        clctNplanesHitPretrig = cms.uint32(2),
        clctNplanesHitPattern = cms.uint32(4),
        clctPidThreshPretrig  = cms.uint32(2),
        clctMinSeparation     = cms.uint32(10),
        # Debug
        verbosity = cms.untracked.int32(0)
    ),

    # Parameters for CLCT processors: MTCC-II
    clctParamMTCC = cms.PSet(
        clctFifoTbins   = cms.uint32(12),
        clctFifoPretrig = cms.uint32(7),
        clctHitPersist  = cms.uint32(6),
        clctDriftDelay  = cms.uint32(2),
        clctNplanesHitPretrig = cms.uint32(4),
        clctNplanesHitPattern = cms.uint32(1),
        clctPidThreshPretrig  = cms.uint32(2),
        clctMinSeparation     = cms.uint32(10),
        # Debug
        verbosity = cms.untracked.int32(0)
    ),

    # Parameters for CLCT processors: 2007 and later
    clctParam07 = cms.PSet(
        clctFifoTbins   = cms.uint32(12),
        clctFifoPretrig = cms.uint32(7),
        clctHitPersist  = cms.uint32(6),
        clctDriftDelay  = cms.uint32(2),
        clctNplanesHitPretrig = cms.uint32(2),
        clctNplanesHitPattern = cms.uint32(4),
        clctPidThreshPretrig  = cms.uint32(2),
        clctMinSeparation     = cms.uint32(10),
        # Debug
        verbosity = cms.untracked.int32(0),

        # BX to start CLCT finding
        clctStartBxShift  = cms.untracked.int32(0)
    ),

    # Parameters for CLCT processors: SLHC studies
    clctSLHC = cms.PSet(
        clctFifoTbins   = cms.uint32(12),
        clctFifoPretrig = cms.uint32(7),
        clctHitPersist  = cms.uint32(6),
        clctDriftDelay  = cms.uint32(2),
        clctNplanesHitPretrig = cms.uint32(2),
        clctNplanesHitPattern = cms.uint32(4),
        clctPidThreshPretrig  = cms.uint32(2),
        clctMinSeparation     = cms.uint32(8),
        # Debug
        verbosity = cms.untracked.int32(0),

        # poor man's deadtime shortening
        clctStartBxShift  = cms.untracked.int32(0),
        # use of localized dead-time zones
        useDeadTimeZoning = cms.untracked.bool(True),
        clctStateMachineZone = cms.untracked.uint32(8),
        useDynamicStateMachineZone = cms.untracked.bool(True),
        # pretrigger to trigger matching zone
        clctPretriggerTriggerZone = cms.untracked.uint32(5),
        clctUseCorrectedBx = cms.untracked.bool(False)
    ),

    tmbParam = cms.PSet(
        mpcBlockMe1a    = cms.uint32(1),
        alctTrigEnable  = cms.uint32(0),
        clctTrigEnable  = cms.uint32(0),
        matchTrigEnable = cms.uint32(1),
        matchTrigWindowSize = cms.uint32(3),
        tmbL1aWindowSize = cms.uint32(3),
        # Debug
        verbosity = cms.untracked.int32(0),

        tmbEarlyTbins = cms.untracked.int32(4),
        clctToAlct = cms.untracked.bool(True),
        tmbDropUsedAlcts = cms.untracked.bool(False),
        tmbDropUsedClcts = cms.untracked.bool(False),
        matchEarliestAlctME11Only = cms.untracked.bool(True),
        matchEarliestClctME11Only = cms.untracked.bool(True),
        tmbCrossBxAlgorithm = cms.untracked.uint32(0),
        maxME11LCTs = cms.untracked.uint32(4),
        tmbReadoutEarliest2 = cms.untracked.bool(False)
    ),

    # MPC sorter config for SLHC studies
    mpcSLHC = cms.PSet(
        mpcMaxStubs = cms.untracked.uint32(18)
    )
)
