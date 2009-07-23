//-----------------------------------------------------------------------------
//
//   Class: CSCMotherboardME11
//
//   Description:
//    Extended CSCMotherboard for ME11 to handle ME1a and ME1b separately
//
//   Author List: Vadim Khotilovich 12 May 2009
//
//   $Date: 2009/05/12 03:31:02 $
//   $Revision: 1.21.2.2 $
//
//-----------------------------------------------------------------------------

#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboardME11.h>
//#include <Utilities/Timing/interface/TimingReport.h>
#include <FWCore/MessageLogger/interface/MessageLogger.h>
#include <DataFormats/MuonDetId/interface/CSCTriggerNumbering.h>



// LUT for which ME1/1 wire group can cross which ME1/a halfstrip
// 1st index: WG number
// 2nd index: inclusive HS range
const int CSCMotherboardME11::lut_wg_vs_hs_me1a[48][2] = {
{0, 95},{0, 95},{0, 95},{0, 95},{0, 95},
{0, 95},{0, 95},{0, 95},{0, 95},{0, 95},
{0, 95},{0, 95},{0, 77},{0, 61},{0, 39},
{0, 22},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1} };

// LUT for which ME1/1 wire group can cross which ME1/b halfstrip
// 1st index: WG number
// 2nd index: inclusive HS range
const int CSCMotherboardME11::lut_wg_vs_hs_me1b[48][2] = {
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
{100, 127},{73, 127},{47, 127},{22, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 127},
{0, 127},{0, 127},{0, 127},{0, 127},{0, 105},
{0, 93},{0, 78},{0, 63} };


CSCMotherboardME11::CSCMotherboardME11(unsigned endcap, unsigned station,
			       unsigned sector, unsigned subsector,
			       unsigned chamber,
			       const edm::ParameterSet& conf) :
		CSCMotherboard(endcap, station, sector, subsector, 
		               chamber, conf) {
  edm::ParameterSet commonParams =
    conf.getParameter<edm::ParameterSet>("commonParam");
  edm::ParameterSet alctParams = 
    conf.getParameter<edm::ParameterSet>("alctSLHC");
  edm::ParameterSet clctParams = 
    conf.getParameter<edm::ParameterSet>("clctSLHC");
  edm::ParameterSet tmbParams =
    conf.getParameter<edm::ParameterSet>("tmbParam");

  alct1a = new CSCAnodeLCTProcessor(endcap, station, sector, subsector,
				  chamber, alctParams, commonParams);
  clct1a = new CSCCathodeLCTProcessor(endcap, station, sector, subsector,
				    chamber, clctParams, commonParams, tmbParams);
  alct1a->setRing(4);
  clct1a->setRing(4);
}

CSCMotherboardME11::CSCMotherboardME11() : CSCMotherboard() {
  // Constructor used only for testing.

  alct1a = new CSCAnodeLCTProcessor();
  clct1a = new CSCCathodeLCTProcessor();
  alct1a->setRing(4);
  clct1a->setRing(4);
}

CSCMotherboardME11::~CSCMotherboardME11() {
  if (alct1a) delete alct1a;
  if (clct1a) delete clct1a;
}

void CSCMotherboardME11::clear() {
  CSCMotherboard::clear();
  if (alct1a) alct1a->clear();
  if (clct1a) clct1a->clear();
  for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
    firstLCT1a[bx].clear();
    secondLCT1a[bx].clear();
  }
}

// Set configuration parameters obtained via EventSetup mechanism.
void CSCMotherboardME11::setConfigParameters(const CSCL1TPParameters* conf) {
  alct->setConfigParameters(conf);
  clct->setConfigParameters(conf);
  alct1a->setConfigParameters(conf);
  clct1a->setConfigParameters(conf);
  // No config. parameters in DB for the TMB itself yet.
}


void
CSCMotherboardME11::runME11(const CSCWireDigiCollection* wiredc,
		    const CSCComparatorDigiCollection* compdc) {
  clear();

  int n_clct_a=0, n_clct_b=0;
  
  if (alct && clct && !smartME1aME1b) {
    alctV = alct->run(wiredc); // run anodeLCT
    clctV = clct->run(compdc); // run cathodeLCT

    if (clct->bestCLCT[6].isValid() && clct->bestCLCT[6].getBX()==6) n_clct_b++;
    if (clct->secondCLCT[6].isValid() && clct->secondCLCT[6].getBX()==6) n_clct_b++;

    for (int bx_clct = 0; bx_clct < CSCCathodeLCTProcessor::MAX_CLCT_BINS; bx_clct++) {
      if (clct->bestCLCT[bx_clct].isValid()) {
	int bx_alct_start = bx_clct - match_trig_window_size/2;
	int bx_alct_stop  = bx_clct + match_trig_window_size/2;
	for (int bx_alct = bx_alct_start; bx_alct <= bx_alct_stop; bx_alct++) {
	  if (bx_alct < 0 || bx_alct >= CSCAnodeLCTProcessor::MAX_ALCT_BINS) continue;
	  if (alct->bestALCT[bx_alct].isValid()) {
	    if (infoV > 1) LogTrace("CSCMotherboard")
	      << "Successful ALCT-CLCT match in ME1b: bx_clct = " << bx_clct
	      << "; match window: [" << bx_alct_start << "; " << bx_alct_stop
	      << "]; bx_alct = " << bx_alct;
	    correlateLCTs(alct->bestALCT[bx_alct], alct->secondALCT[bx_alct],
			  clct->bestCLCT[bx_clct], clct->secondCLCT[bx_clct],
			  firstLCT[bx_alct], secondLCT[bx_alct]);
	    break;
	  }
	}
	// Do not report CLCT-only LCT for ME1b
      }
      // Do not attempt to make ALCT-only LCT for ME1b
    }

    if (infoV > 0) {
      for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
	if (firstLCT[bx].isValid()) LogDebug("CSCMotherboard") << firstLCT[bx];
	if (secondLCT[bx].isValid()) LogDebug("CSCMotherboard") << secondLCT[bx];
      }
    }
  }
  if (alct1a && clct1a && naiveME1aME1b && !smartME1aME1b && !disableME1a) {
    alctV1a = alct1a->run(wiredc); // run anodeLCT
    clctV1a = clct1a->run(compdc); // run cathodeLCT

    if (clct1a->bestCLCT[6].isValid() && clct1a->bestCLCT[6].getBX()==6) n_clct_a++;
    if (clct1a->secondCLCT[6].isValid() && clct1a->secondCLCT[6].getBX()==6) n_clct_a++;

    for (int bx_clct = 0; bx_clct < CSCCathodeLCTProcessor::MAX_CLCT_BINS; bx_clct++) {
      if (clct1a->bestCLCT[bx_clct].isValid()) {
	int bx_alct_start = bx_clct - match_trig_window_size/2;
	int bx_alct_stop  = bx_clct + match_trig_window_size/2;
	for (int bx_alct = bx_alct_start; bx_alct <= bx_alct_stop; bx_alct++) {
	  if (bx_alct < 0 || bx_alct >= CSCAnodeLCTProcessor::MAX_ALCT_BINS) continue;
	  if (alct1a->bestALCT[bx_alct].isValid()) {
	    if (infoV > 1) LogTrace("CSCMotherboard")
	      << "Successful ALCT-CLCT match in ME1a: bx_clct = " << bx_clct
	      << "; match window: [" << bx_alct_start << "; " << bx_alct_stop
	      << "]; bx_alct = " << bx_alct;
	    correlateLCTs(alct1a->bestALCT[bx_alct], alct1a->secondALCT[bx_alct],
			  clct1a->bestCLCT[bx_clct], clct1a->secondCLCT[bx_clct],
			  firstLCT1a[bx_alct], secondLCT1a[bx_alct]);
	    break;
	  }
	}
	// Do not report CLCT-only LCT for ME1b
      }
      // Do not attempt to make ALCT-only LCT for ME1b
    }

    if (infoV > 0) {
      for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
	if (firstLCT1a[bx].isValid()) LogDebug("CSCMotherboard") << firstLCT[bx];
	if (secondLCT1a[bx].isValid()) LogDebug("CSCMotherboard") << secondLCT[bx];
      }
    }
  }
  if ( alct && clct &&  clct1a && smartME1aME1b && !naiveME1aME1b && !disableME1a) {
    alctV = alct->run(wiredc); // run anodeLCT
    clctV = clct->run(compdc); // run cathodeLCT in ME1/b
    clctV1a = clct1a->run(compdc); // run cathodeLCT in ME1/a

    //if (clct1a->bestCLCT[6].isValid() && clct1a->bestCLCT[6].getBX()==6) n_clct_a++;
    //if (clct1a->secondCLCT[6].isValid() && clct1a->secondCLCT[6].getBX()==6) n_clct_a++;

    for (int bx_clct = 0; bx_clct < CSCCathodeLCTProcessor::MAX_CLCT_BINS; bx_clct++) {
      
      if (clct->bestCLCT[bx_clct].isValid()) {
	int bx_alct_start = bx_clct - match_trig_window_size/2;
	int bx_alct_stop  = bx_clct + match_trig_window_size/2;
	for (int bx_alct = bx_alct_start; bx_alct <= bx_alct_stop; bx_alct++) {
	  if (bx_alct < 0 || bx_alct >= CSCAnodeLCTProcessor::MAX_ALCT_BINS) continue;
	  if (alct->bestALCT[bx_alct].isValid()) {
	    if (infoV > 1) LogTrace("CSCMotherboard")
	      << "Successful ALCT-CLCT match in ME1b: bx_clct = " << bx_clct
	      << "; match window: [" << bx_alct_start << "; " << bx_alct_stop
	      << "]; bx_alct = " << bx_alct;
	    correlateLCTs(alct->bestALCT[bx_alct], alct->secondALCT[bx_alct],
			  clct->bestCLCT[bx_clct], clct->secondCLCT[bx_clct],
			  firstLCT[bx_alct], secondLCT[bx_alct], ME1B);
	    break;
	  }
	}
	// Do not report CLCT-only LCT for ME1b
      }
      if (clct1a->bestCLCT[bx_clct].isValid()) {
	int bx_alct_start = bx_clct - match_trig_window_size/2;
	int bx_alct_stop  = bx_clct + match_trig_window_size/2;
	for (int bx_alct = bx_alct_start; bx_alct <= bx_alct_stop; bx_alct++) {
	  if (bx_alct < 0 || bx_alct >= CSCAnodeLCTProcessor::MAX_ALCT_BINS) continue;
	  if (alct->bestALCT[bx_alct].isValid()) {
	    if (infoV > 1) LogTrace("CSCMotherboard")
	      << "Successful ALCT-CLCT match in ME1a: bx_clct = " << bx_clct
	      << "; match window: [" << bx_alct_start << "; " << bx_alct_stop
	      << "]; bx_alct = " << bx_alct;
	    correlateLCTs(alct->bestALCT[bx_alct], alct->secondALCT[bx_alct],
			  clct1a->bestCLCT[bx_clct], clct1a->secondCLCT[bx_clct],
			  firstLCT1a[bx_alct], secondLCT1a[bx_alct], ME1A);
	    break;
	  }
	}
	// Do not report CLCT-only LCT for ME1b
      }
      // Do not attempt to make ALCT-only LCT for ME1b
    }

    if (infoV > 0) {
      for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
	if (firstLCT[bx].isValid()) LogDebug("CSCMotherboard") << "1B LCT1 "<<bx<<": "<<firstLCT[bx];
	if (secondLCT[bx].isValid()) LogDebug("CSCMotherboard") << "1B LCT2 "<<bx<<": "<<secondLCT[bx];
	if (firstLCT1a[bx].isValid()) LogDebug("CSCMotherboard") << "1a LCT1 "<<bx<<": "<<firstLCT[bx];
	if (secondLCT1a[bx].isValid()) LogDebug("CSCMotherboard") << "1a LCT2 "<<bx<<": "<<secondLCT[bx];
      }
    }
    
    if (max_me11_lcts==2){
      for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
        int nlcts=0;
        if (firstLCT[bx].isValid()) nlcts++;
        if (secondLCT[bx].isValid()) nlcts++;
        if (firstLCT1a[bx].isValid()) nlcts++;
        if (secondLCT1a[bx].isValid()) nlcts++;
        if ( nlcts<3 ) continue;
        // do it simple so far: prefer higer eta
        if (firstLCT[bx].isValid() && secondLCT[bx].isValid()) {
          firstLCT1a[bx].clear();
          secondLCT1a[bx].clear();
        }
        else if (firstLCT[bx].isValid() && firstLCT1a[bx].isValid()) {
          secondLCT[bx].clear();
          secondLCT1a[bx].clear();
        }
        else {
          edm::LogWarning("CSCMotherboard11")
            << "impossible ME11 LCT combination: " 
            <<firstLCT[bx].isValid()<<" "<<secondLCT[bx].isValid()<<" "
            <<firstLCT1a[bx].isValid()<<" "<<secondLCT1a[bx].isValid()<<" \n";
        }
        
      }
    }
  }
  LogTrace("CSCMotherboardME11")<<"clct_count E:"<<theEndcap<<"S:"<<theStation<<"R:"<<1<<"C:"
    <<CSCTriggerNumbering::chamberFromTriggerLabels(theSector,theSubsector, theStation, theTrigChamber)
    <<"  a "<<n_clct_a<<"  b "<<n_clct_b<<"  ab "<<n_clct_a+n_clct_b;

}


std::vector<CSCCorrelatedLCTDigi> CSCMotherboardME11::readoutLCTs1a() {
  return readoutLCTs(ME1A);
}


std::vector<CSCCorrelatedLCTDigi> CSCMotherboardME11::readoutLCTs1b() {
  return readoutLCTs(ME1B);
}


// Returns vector of read-out correlated LCTs, if any.  Starts with
// the vector of all found LCTs and selects the ones in the read-out
// time window.
std::vector<CSCCorrelatedLCTDigi> CSCMotherboardME11::readoutLCTs(int me1ab) {
  std::vector<CSCCorrelatedLCTDigi> tmpV;

  // The start time of the L1A*LCT coincidence window should be related
  // to the fifo_pretrig parameter, but I am not completely sure how.
  // Just choose it such that the window is centered at bx=7.  This may
  // need further tweaking if the value of tmb_l1a_window_size changes.
  //static int early_tbins = 4;
  static int early_tbins = early_tbins_my;
  // The number of LCT bins in the read-out is given by the
  // tmb_l1a_window_size parameter, forced to be odd
  static int lct_bins   = 
    (tmb_l1a_window_size%2 == 0) ? tmb_l1a_window_size+1 : tmb_l1a_window_size;
  static int late_tbins = early_tbins + lct_bins;


  // Start from the vector of all found correlated LCTs and select
  // those within the LCT*L1A coincidence window.
  int bx_readout = -1;
  std::vector<CSCCorrelatedLCTDigi> all_lcts;
  if (me1ab == ME1A) all_lcts = getLCTs1a();
  if (me1ab == ME1B) all_lcts = getLCTs1b();
  for (std::vector <CSCCorrelatedLCTDigi>::const_iterator plct =
	 all_lcts.begin(); plct != all_lcts.end(); plct++) {
    if (!plct->isValid()) continue;

    int bx = (*plct).getBX();
    // Skip LCTs found too early relative to L1Accept.
    if (bx <= early_tbins) {
      continue;
    }

    // Skip LCTs found too late relative to L1Accept.
    if (bx > late_tbins) {
      continue;
    }

    // If (readout_earliest_2) take only LCTs in the earliest bx in the read-out window:
    // in digi->raw step, LCTs have to be packed into the TMB header, and
    // currently there is room just for two.
    if (readout_earliest_2 && (bx_readout == -1 || bx == bx_readout) ) {
      tmpV.push_back(*plct);
      if (bx_readout == -1) bx_readout = bx;
    }
    else tmpV.push_back(*plct);
  }
  return tmpV;
}


// Returns vector of found correlated LCTs, if any.
std::vector<CSCCorrelatedLCTDigi> CSCMotherboardME11::getLCTs1b() {
  std::vector<CSCCorrelatedLCTDigi> tmpV;

  for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
    if (mpc_block_me1a && !naiveME1aME1b && !smartME1aME1b && !disableME1a ) {
      // default emulator case
      // Do not report LCTs found in ME1/A.
      if (firstLCT[bx].isValid())
        if (!mpc_block_me1a || firstLCT[bx].getStrip() <= 127)
	  tmpV.push_back(firstLCT[bx]);
      if (secondLCT[bx].isValid())
        if (!mpc_block_me1a || secondLCT[bx].getStrip() <= 127)
	  tmpV.push_back(secondLCT[bx]);
    } else {
      // Report all LCTs found.
      if (firstLCT[bx].isValid())  tmpV.push_back(firstLCT[bx]);
      if (secondLCT[bx].isValid()) tmpV.push_back(secondLCT[bx]);
    }
  }
  return tmpV;
}

// Returns vector of found correlated LCTs, if any.
std::vector<CSCCorrelatedLCTDigi> CSCMotherboardME11::getLCTs1a() {
  std::vector<CSCCorrelatedLCTDigi> tmpV;
  
  // default case of Do not report LCTs found in ME1/A.
  if (mpc_block_me1a && !naiveME1aME1b && !smartME1aME1b && !disableME1a )
    return tmpV;
  
  // disabled ME1a
  if (disableME1a) return tmpV;

  // Report all LCTs found.
  for (int bx = 0; bx < MAX_LCT_BINS; bx++) {
    if (firstLCT1a[bx].isValid())  tmpV.push_back(firstLCT1a[bx]);
    if (secondLCT1a[bx].isValid()) tmpV.push_back(secondLCT1a[bx]);
  }

  return tmpV;
}

bool CSCMotherboardME11::doesALCTCrossCLCT(CSCALCTDigi &a, CSCCLCTDigi &c, int me) {
  if ( !c.isValid() || !a.isValid() ) return false;
  int key_hs = c.getKeyStrip();
  int key_wg = a.getKeyWG();
  if ( me == ME1A &&
       ( key_hs >= lut_wg_vs_hs_me1a[key_wg][0] || 
         key_hs <= lut_wg_vs_hs_me1a[key_wg][1]    ) ) return true;
  if ( me == ME1B &&
       ( key_hs >= lut_wg_vs_hs_me1b[key_wg][0] || 
         key_hs <= lut_wg_vs_hs_me1b[key_wg][1]    ) ) return true;
  return false;
}


void CSCMotherboardME11::correlateLCTs(CSCALCTDigi bestALCT,
				   CSCALCTDigi secondALCT,
				   CSCCLCTDigi bestCLCT,
				   CSCCLCTDigi secondCLCT,
				   CSCCorrelatedLCTDigi& lct1,
				   CSCCorrelatedLCTDigi& lct2) {
  bool anodeBestValid     = bestALCT.isValid();
  bool anodeSecondValid   = secondALCT.isValid();
  bool cathodeBestValid   = bestCLCT.isValid();
  bool cathodeSecondValid = secondCLCT.isValid();

  if (anodeBestValid && !anodeSecondValid)     secondALCT = bestALCT;
  if (!anodeBestValid && anodeSecondValid)     bestALCT   = secondALCT;
  if (cathodeBestValid && !cathodeSecondValid) secondCLCT = bestCLCT;
  if (!cathodeBestValid && cathodeSecondValid) bestCLCT   = secondCLCT;

  // ALCT-CLCT matching conditions are defined by "trig_enable" configuration
  // parameters.
  if ((alct_trig_enable  && bestALCT.isValid()) ||
      (clct_trig_enable  && bestCLCT.isValid()) ||
      (match_trig_enable && bestALCT.isValid() && bestCLCT.isValid())) {
    lct1 = constructLCTs(bestALCT, bestCLCT);
    lct1.setTrknmb(1);
  }

  if (((secondALCT != bestALCT) || (secondCLCT != bestCLCT)) &&
      ((alct_trig_enable  && secondALCT.isValid()) ||
       (clct_trig_enable  && secondCLCT.isValid()) ||
       (match_trig_enable && secondALCT.isValid() && secondCLCT.isValid()))) {
    lct2 = constructLCTs(secondALCT, secondCLCT);
    lct2.setTrknmb(2);
  }
}

void CSCMotherboardME11::correlateLCTs(CSCALCTDigi bestALCT,
				   CSCALCTDigi secondALCT,
				   CSCCLCTDigi bestCLCT,
				   CSCCLCTDigi secondCLCT,
				   CSCCorrelatedLCTDigi& lct1,
				   CSCCorrelatedLCTDigi& lct2,
                                   int me) {

  // assume that always anodeBestValid && cathodeBestValid
  
  if (secondALCT == bestALCT) secondALCT.clear();
  if (secondCLCT == bestCLCT) secondCLCT.clear();
  
  int ok11 = doesALCTCrossCLCT( bestALCT, bestCLCT, me);
  int ok12 = doesALCTCrossCLCT( bestALCT, secondCLCT, me);
  int ok21 = doesALCTCrossCLCT( secondALCT, bestCLCT, me);
  int ok22 = doesALCTCrossCLCT( secondALCT, secondCLCT, me);
  int code = (ok11<<3) | (ok12<<2) | (ok21<<1) | (ok22);
  
  if ( code==0 ) return;

  // LUT defines correspondence between possible ok## combinations
  // and resulting lct1 and lct2
  int lut[16][2] = {
          //ok: 11 12 21 22
    {0 ,0 }, // 0  0  0  0
    {22,0 }, // 0  0  0  1
    {21,0 }, // 0  0  1  0
    {21,22}, // 0  0  1  1
    {12,0 }, // 0  1  0  0
    {12,22}, // 0  1  0  1
    {12,21}, // 0  1  1  0
    {12,21}, // 0  1  1  1
    {11,0 }, // 1  0  0  0
    {11,22}, // 1  0  0  1
    {11,21}, // 1  0  1  0
    {11,22}, // 1  0  1  1
    {11,12}, // 1  1  0  0
    {11,22}, // 1  1  0  1
    {11,12}, // 1  1  1  0
    {11,22}, // 1  1  1  1
  };
  
  switch (lut[code][0]) {
    case 11:
      lct1 = constructLCTs(bestALCT, bestCLCT);
      break;
    case 12:
      lct1 = constructLCTs(bestALCT, secondCLCT);
      break;
    case 21:
      lct1 = constructLCTs(secondALCT, bestCLCT);
      break;
    case 22:
      lct1 = constructLCTs(secondALCT, secondCLCT);
      break;
    default: return;  
  }
  lct1.setTrknmb(1);
  
  switch (lut[code][1]) {
    case 12:
      lct2 = constructLCTs(bestALCT, secondCLCT);
      lct2.setTrknmb(2);
      return;
    case 21:
      lct2 = constructLCTs(secondALCT, bestCLCT);
      lct2.setTrknmb(2);
      return;
    case 22:
      lct2 = constructLCTs(secondALCT, secondCLCT);
      lct2.setTrknmb(2);
      return;
    default: return;
  }
  return;
}

