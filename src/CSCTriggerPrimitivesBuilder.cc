//-----------------------------------------------------------------------------
//
//   Class: CSCTriggerPrimitivesBuilder
//
//   Description: Algorithm to build anode, cathode, and correlated LCTs
//                in each endcap muon CSC chamber from wire and comparator
//                digis.
//
//   Author List: S. Valuev, UCLA.
//
//   $Date: 2009/05/20 14:59:13 $
//   $Revision: 1.17 $
//
//   Modifications:
//
//-----------------------------------------------------------------------------

#include <L1Trigger/CSCTriggerPrimitives/src/CSCTriggerPrimitivesBuilder.h>
#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboard.h>
#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboardME11.h>
#include <L1Trigger/CSCTriggerPrimitives/src/CSCMuonPortCard.h>

#include <FWCore/MessageLogger/interface/MessageLogger.h>

#include <L1Trigger/CSCCommonTrigger/interface/CSCTriggerGeometry.h>
#include <DataFormats/MuonDetId/interface/CSCTriggerNumbering.h>
#include <DataFormats/MuonDetId/interface/CSCDetId.h>

//------------------
// Static variables
//------------------
const int CSCTriggerPrimitivesBuilder::min_endcap  = CSCDetId::minEndcapId();
const int CSCTriggerPrimitivesBuilder::max_endcap  = CSCDetId::maxEndcapId();
const int CSCTriggerPrimitivesBuilder::min_station = CSCDetId::minStationId();
const int CSCTriggerPrimitivesBuilder::max_station = CSCDetId::maxStationId();
const int CSCTriggerPrimitivesBuilder::min_sector  =
                                  CSCTriggerNumbering::minTriggerSectorId();
const int CSCTriggerPrimitivesBuilder::max_sector  =
                                  CSCTriggerNumbering::maxTriggerSectorId();
const int CSCTriggerPrimitivesBuilder::min_subsector =
                                  CSCTriggerNumbering::minTriggerSubSectorId();
const int CSCTriggerPrimitivesBuilder::max_subsector =
                                  CSCTriggerNumbering::maxTriggerSubSectorId();
const int CSCTriggerPrimitivesBuilder::min_chamber =
                                  CSCTriggerNumbering::minTriggerCscId();
const int CSCTriggerPrimitivesBuilder::max_chamber =
                                  CSCTriggerNumbering::maxTriggerCscId();

//-------------
// Constructor
//-------------
CSCTriggerPrimitivesBuilder::CSCTriggerPrimitivesBuilder(const edm::ParameterSet& conf) {
  // Receives ParameterSet percolated down from EDProducer.

  // ORCA way of initializing boards.
  for (int endc = min_endcap; endc <= max_endcap; endc++) {
    for (int stat = min_station; stat <= max_station; stat++) {
      int numsubs = ((stat == 1) ? max_subsector : 1);
      for (int sect = min_sector; sect <= max_sector; sect++) {
	for (int subs = min_subsector; subs <= numsubs; subs++) {
	  for (int cham = min_chamber; cham <= max_chamber; cham++) {
	    if ((endc <= 0 || endc > MAX_ENDCAPS)    ||
		(stat <= 0 || stat > MAX_STATIONS)   ||
		(sect <= 0 || sect > MAX_SECTORS)    ||
		(subs <= 0 || subs > MAX_SUBSECTORS) ||
		(cham <= 0 || stat > MAX_CHAMBERS)) {
	      edm::LogError("CSCTriggerPrimitivesBuilder")
		<< "+++ trying to instantiate TMB of illegal CSC id ["
		<< " endcap = "  << endc << " station = "   << stat
		<< " sector = "  << sect << " subsector = " << subs
		<< " chamber = " << cham << "]; skipping it... +++\n";
	      continue;
	    }
	    int ring = CSCTriggerNumbering::ringFromTriggerLabels(stat, cham);
	    // When the motherboard is instantiated, it instantiates ALCT
	    // and CLCT processors.
           if (stat==1 && ring==1)
              tmb_[endc-1][stat-1][sect-1][subs-1][cham-1] =
               new CSCMotherboardME11(endc, stat, sect, subs, cham, conf);
           else
             tmb_[endc-1][stat-1][sect-1][subs-1][cham-1] =
               new CSCMotherboard(endc, stat, sect, subs, cham, conf);
	  }
	}
      }
    }
  }

  // special configuration parameters for ME11 treatment
  edm::ParameterSet commonParams =
    conf.getParameter<edm::ParameterSet>("commonParam");
  naiveME1aME1b = commonParams.getUntrackedParameter<bool>("naiveME1aME1b",false);
  smartME1aME1b = commonParams.getUntrackedParameter<bool>("smartME1aME1b",false);
  disableME1a = commonParams.getUntrackedParameter<bool>("disableME1a",false);

  
  // Get min and max BX to sort LCTs in MPC.
  m_minBX = conf.getParameter<int>("MinBX");
  m_maxBX = conf.getParameter<int>("MaxBX");
  
  // Init MPC
  m_muonportcard = new CSCMuonPortCard();
}

//------------
// Destructor
//------------
CSCTriggerPrimitivesBuilder::~CSCTriggerPrimitivesBuilder() {
  for (int endc = min_endcap; endc <= max_endcap; endc++) {
    for (int stat = min_station; stat <= max_station; stat++) {
      int numsubs = ((stat == 1) ? max_subsector : 1);
      for (int sect = min_sector; sect <= max_sector; sect++) {
	for (int subs = min_subsector; subs <= numsubs; subs++) {
	  for (int cham = min_chamber; cham <= max_chamber; cham++) {
	    delete tmb_[endc-1][stat-1][sect-1][subs-1][cham-1];
	  }
	}
      }
    }
  }
  delete m_muonportcard;
}

//------------
// Operations
//------------
// Set configuration parameters obtained via EventSetup mechanism.
void CSCTriggerPrimitivesBuilder::setConfigParameters(const CSCL1TPParameters* conf) {
  // Receives CSCL1TPParameters percolated down from ESProducer.

  for (int endc = min_endcap; endc <= max_endcap; endc++) {
    for (int stat = min_station; stat <= max_station; stat++) {
      int numsubs = ((stat == 1) ? max_subsector : 1);
      for (int sect = min_sector; sect <= max_sector; sect++) {
	for (int subs = min_subsector; subs <= numsubs; subs++) {
	  for (int cham = min_chamber; cham <= max_chamber; cham++) {
	    CSCMotherboard* tmb = tmb_[endc-1][stat-1][sect-1][subs-1][cham-1];
	    tmb->setConfigParameters(conf);
	  }
	}
      }
    }
  }
}

// Build anode, cathode, and correlated LCTs in each chamber and fill them
// into output collections.  Pass collections of wire and comparator digis
// to Trigger MotherBoard (TMB) processors, which, in turn, pass them to
// ALCT and CLCT processors.  Up to 2 anode and 2 cathode LCTs can be found
// in each chamber during any bunch crossing.  The 2 projections are then
// combined into three-dimensional "correlated" LCTs in the TMB.  Finally,
// MPC processor sorts up to 18 LCTs from 9 TMBs and writes collections of
// up to 3 best LCTs per (sub)sector into Event (to be used by the Sector
// Receiver).
void CSCTriggerPrimitivesBuilder::build(const CSCBadChambers* badChambers,
					const CSCWireDigiCollection* wiredc,
					const CSCComparatorDigiCollection* compdc,
					CSCALCTDigiCollection& oc_alct,
					CSCCLCTDigiCollection& oc_clct,
					CSCCorrelatedLCTDigiCollection& oc_lct,
					CSCCorrelatedLCTDigiCollection& oc_sorted_lct) {
  // CSC geometry.
  CSCTriggerGeomManager* theGeom = CSCTriggerGeometry::get();

  for (int endc = min_endcap; endc <= max_endcap; endc++) {
    for (int stat = min_station; stat <= max_station; stat++) {
      int numsubs = ((stat == 1) ? max_subsector : 1);
      for (int sect = min_sector; sect <= max_sector; sect++) {
	for (int subs = min_subsector; subs <= numsubs; subs++) {
	  for (int cham = min_chamber; cham <= max_chamber; cham++) {
	    int ring = CSCTriggerNumbering::ringFromTriggerLabels(stat, cham);

            //LogTrace("CSCTriggerPrimitivesBuilder")<<"CSCTriggerPrimitivesBuilder::build in E:"<<endc<<" S:"<<stat<<" R:"<<ring;

	    if (!(stat==1 && ring==1)) {
	      CSCMotherboard* tmb = tmb_[endc-1][stat-1][sect-1][subs-1][cham-1];

	      //LogTrace("CSCTriggerPrimitivesBuilder")<<"CSCTriggerPrimitivesBuilder::before geom check E:"<<endc<<" S:"<<stat<<" R:"<<ring;

	      // Run processors only if chamber exists in geometry.
	      if (tmb == 0 ||
		theGeom->chamber(endc, stat, sect, subs, cham) == 0) continue;

	      //LogTrace("CSCTriggerPrimitivesBuilder")<<"CSCTriggerPrimitivesBuilder::before tmb:run E:"<<endc<<" S:"<<stat<<" R:"<<ring;

 	      int chid =
		CSCTriggerNumbering::chamberFromTriggerLabels(sect, subs,
							      stat, cham);
	      // 0th layer means whole chamber.
	      CSCDetId detid(endc, stat, ring, chid, 0);

	      // Skip chambers marked as bad (includes most of ME4/2 chambers).
	      //if (badChambers && badChambers->isInBadChamber(detid)) continue;

	      std::vector<CSCCorrelatedLCTDigi> lctV = tmb->run(wiredc,compdc);

	      std::vector<CSCALCTDigi> alctV = tmb->alct->readoutALCTs();
	      std::vector<CSCCLCTDigi> clctV = tmb->clct->readoutCLCTs();

	      // Skip to next chamber if there are no LCTs to save.
	      if (alctV.empty() && clctV.empty() && lctV.empty()) continue;

	      // Correlated LCTs.
	      if (!lctV.empty()) {
		LogTrace("L1CSCTrigger")
		  << "Put " << lctV.size() << " LCT digi"
		  << ((lctV.size() > 1) ? "s " : " ") << "in collection\n";
		oc_lct.put(std::make_pair(lctV.begin(),lctV.end()), detid);
	      }

	      // Anode LCTs.
	      if (!alctV.empty()) {
		LogTrace("L1CSCTrigger")
		  << "Put " << alctV.size() << " ALCT digi"
		  << ((alctV.size() > 1) ? "s " : " ") << "in collection\n";
		oc_alct.put(std::make_pair(alctV.begin(),alctV.end()), detid);
	      }

	      // Cathode LCTs.
	      if (!clctV.empty()) {
		LogTrace("L1CSCTrigger")
		  << "Put " << clctV.size() << " CLCT digi"
		  << ((clctV.size() > 1) ? "s " : " ") << "in collection\n";
		oc_clct.put(std::make_pair(clctV.begin(),clctV.end()), detid);
	      }
	    }
            else {
              CSCMotherboardME11* tmb = (CSCMotherboardME11*)
                tmb_[endc-1][stat-1][sect-1][subs-1][cham-1];
 
              // Run processors only if chamber exists in geometry.
              if (tmb == 0 ||
                  theGeom->chamber(endc, stat, sect, subs, cham) == 0) continue;
 
              //LogTrace("CSCTriggerPrimitivesBuilder")<<"CSCTriggerPrimitivesBuilder::build in E:"<<endc<<" S:"<<stat<<" R:"<<ring;
 
              tmb->runME11(wiredc,compdc);
              std::vector<CSCCorrelatedLCTDigi> lctV = tmb->readoutLCTs1b();
              std::vector<CSCCorrelatedLCTDigi> lctV1a = tmb->readoutLCTs1a();
 
              std::vector<CSCALCTDigi> alctV = tmb->alct->readoutALCTs();
              std::vector<CSCCLCTDigi> clctV = tmb->clct->readoutCLCTs();
              std::vector<CSCALCTDigi> alctV1a = tmb->alct1a->readoutALCTs();
              std::vector<CSCCLCTDigi> clctV1a = tmb->clct1a->readoutCLCTs();
 
              // perform simple separation of ALCTs into 1/a and 1/b
              // for 'smart' case. Some duplication occurs for WG [10,15]
              if (smartME1aME1b && !naiveME1aME1b) {
                std::vector<CSCALCTDigi> tmpV(alctV);
                alctV.clear();
                alctV1a.clear();
                for (unsigned int al=0; al<tmpV.size(); al++){
                  if (tmpV[al].getKeyWG()<=15) alctV1a.push_back(tmpV[al]);
                  if (tmpV[al].getKeyWG()>=10) alctV.push_back(tmpV[al]);
                }
              }
              
              //LogTrace("CSCTriggerPrimitivesBuilder")<<"CSCTriggerPrimitivesBuilder:: a="<<alctV.size()<<" c="<<clctV.size()<<" l="<<lctV.size()
                //<<"   1a: a="<<alctV1a.size()<<" c="<<clctV1a.size()<<" l="<<lctV1a.size();
 
              int chid =
                CSCTriggerNumbering::chamberFromTriggerLabels(sect, subs,
                                                              stat, cham);
              CSCDetId detid(endc, stat, 1, chid, 0);

              // Skip chambers marked as bad
              //if (badChambers && badChambers->isInBadChamber(detid)) continue;

              // Correlated LCTs.
              if (!lctV.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << lctV.size() << " ME1b LCT digi"
                  << ((lctV.size() > 1) ? "s " : " ") << "in collection\n";
                oc_lct.put(std::make_pair(lctV.begin(),lctV.end()), detid);
              }
 
              // Anode LCTs.
              if (!alctV.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << alctV.size() << " ME1b ALCT digi"
                  << ((alctV.size() > 1) ? "s " : " ") << "in collection\n";
                oc_alct.put(std::make_pair(alctV.begin(),alctV.end()), detid);
              }
 
              // Cathode LCTs.
              if (!clctV.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << clctV.size() << " ME1b CLCT digi"
                  << ((clctV.size() > 1) ? "s " : " ") << "in collection\n";
                oc_clct.put(std::make_pair(clctV.begin(),clctV.end()), detid);
              }
 
              CSCDetId detid1a(endc, stat, 4, chid, 0);

              // we know nothing about  bad chambers in ME1a so far
              //if (badChambers && badChambers->isInBadChamber(detid)) continue;
 
              // Correlated LCTs.
              if (!lctV1a.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << lctV1a.size() << " ME1a LCT digi"
                  << ((lctV1a.size() > 1) ? "s " : " ") << "in collection\n";
                oc_lct.put(std::make_pair(lctV1a.begin(),lctV1a.end()), detid1a);
              }
 
              // Anode LCTs.
              if (!alctV1a.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << alctV1a.size() << " ME1a ALCT digi"
                  << ((alctV1a.size() > 1) ? "s " : " ") << "in collection\n";
                oc_alct.put(std::make_pair(alctV1a.begin(),alctV1a.end()), detid1a);
              }
 
              // Cathode LCTs.
              if (!clctV1a.empty()) {
                LogTrace("L1CSCTrigger")
                  << "Put " << clctV1a.size() << " ME1a CLCT digi"
                  << ((clctV1a.size() > 1) ? "s " : " ") << "in collection\n";
                oc_clct.put(std::make_pair(clctV1a.begin(),clctV1a.end()), detid1a);
              }
            }
	  }
	}
      }
    }
  }

  // run MPC simulation
  m_muonportcard->loadDigis(oc_lct);

  std::vector<csctf::TrackStub> result;
  for(int bx = m_minBX; bx <= m_maxBX; ++bx)
    for(int e = min_endcap; e <= max_endcap; ++e)
      for(int st = min_station; st <= max_station; ++st)
        for(int se = min_sector; se <= max_sector; ++se)
          {
            if(st == 1)
              {
		std::vector<csctf::TrackStub> subs1, subs2;
                subs1 = m_muonportcard->sort(e, st, se, 1, bx);
                subs2 = m_muonportcard->sort(e, st, se, 2, bx);
                result.insert(result.end(), subs1.begin(), subs1.end());
                result.insert(result.end(), subs2.begin(), subs2.end());
              }
            else
              {
		std::vector<csctf::TrackStub> sector;
                sector = m_muonportcard->sort(e, st, se, 0, bx);
                result.insert(result.end(), sector.begin(), sector.end());
              }
          }

  std::vector<csctf::TrackStub>::const_iterator itr = result.begin();
  for (; itr != result.end(); itr++) {
    oc_sorted_lct.insertDigi(CSCDetId(itr->getDetId().rawId()), *(itr->getDigi()));
    LogDebug("L1CSCTrigger")
      << "MPC " << *(itr->getDigi()) << " found in ME"
      << ((itr->endcap() == 1) ? "+" : "-") << itr->station() << "/"
      << CSCDetId(itr->getDetId().rawId()).ring() << "/"
      << CSCDetId(itr->getDetId().rawId()).chamber()
      << " (sector " << itr->sector()
      << " trig id. " << itr->cscid() << ")" << "\n";
  }
}
