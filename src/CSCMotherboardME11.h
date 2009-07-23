#ifndef CSCTriggerPrimitives_CSCMotherboardME11_h
#define CSCTriggerPrimitives_CSCMotherboardME11_h

/** \class CSCMotherboardME11
 *
 * Extended CSCMotherboard for ME11 to handle ME1a and ME1b separately
 *
 * \author Vadim Khotilovich 12 May 2009
 *
 * Based on CSCMotherboard code
 *
 * $Date: 2009/05/12 03:31:02 $
 * $Revision: 1.9.2.2 $
 *
 */

#include <L1Trigger/CSCTriggerPrimitives/src/CSCMotherboard.h>
#include <DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigi.h>

class CSCMotherboardME11 : public CSCMotherboard
{
 public:
  /** Normal constructor. */
  CSCMotherboardME11(unsigned endcap, unsigned station, unsigned sector, 
		 unsigned subsector, unsigned chamber,
		 const edm::ParameterSet& conf);

  /** Constructor for use during testing. */
  CSCMotherboardME11();

  /** Default destructor. */
  ~CSCMotherboardME11();

  /** Run function for normal usage.  Runs cathode and anode LCT processors,
      takes results and correlates into CorrelatedLCT. */
  void runME11(const CSCWireDigiCollection* wiredc,
	       const CSCComparatorDigiCollection* compdc);

  /** Returns vectors of found correlated LCTs in ME1a and ME1b, if any. */
  std::vector<CSCCorrelatedLCTDigi> getLCTs1a();
  std::vector<CSCCorrelatedLCTDigi> getLCTs1b();

  /** Returns vectors of found ALCTs in ME1a and ME1b, if any. */
  std::vector<CSCALCTDigi> getALCTs1a() {return alctV1a;}
  std::vector<CSCALCTDigi> getALCTs1b() {return alctV;}

  /** Returns vectors of found CLCTs in ME1a and ME1b, if any. */
  std::vector<CSCCLCTDigi> getCLCTs1a() {return clctV1a;}
  std::vector<CSCCLCTDigi> getCLCTs1b() {return clctV;}

  /** Clears correlated LCT and passes clear signal on to cathode and anode
      LCT processors. */
  void clear();

  /** Set configuration parameters obtained via EventSetup mechanism. */
  void setConfigParameters(const CSCL1TPParameters* conf);

  /** additional Anode LCT processor for ME1a */
  CSCAnodeLCTProcessor* alct1a;

  /** additional Cathode LCT processor for ME1a */
  CSCCathodeLCTProcessor* clct1a;

  std::vector<CSCCorrelatedLCTDigi> readoutLCTs1a();
  std::vector<CSCCorrelatedLCTDigi> readoutLCTs1b();
  std::vector<CSCCorrelatedLCTDigi> readoutLCTs(int me1ab);

 private:

  /** VK: enum labels for ME1a and ME1B */
  enum {ME1B = 1, ME1A=4};

  static const int lut_wg_vs_hs_me1a[48][2];
  static const int lut_wg_vs_hs_me1b[48][2];
  
  bool doesALCTCrossCLCT(CSCALCTDigi &a, CSCCLCTDigi &c, int me);

  /** Container for first correlated LCT in ME1a. */
  CSCCorrelatedLCTDigi firstLCT1a[MAX_LCT_BINS];

  /** Container for second correlated LCT in ME1a. */
  CSCCorrelatedLCTDigi secondLCT1a[MAX_LCT_BINS];

  void correlateLCTs(CSCALCTDigi bestALCT, CSCALCTDigi secondALCT,
		     CSCCLCTDigi bestCLCT, CSCCLCTDigi secondCLCT,
		     CSCCorrelatedLCTDigi& lct1, CSCCorrelatedLCTDigi& lct2);

  void correlateLCTs(CSCALCTDigi bestALCT, CSCALCTDigi secondALCT,
		     CSCCLCTDigi bestCLCT, CSCCLCTDigi secondCLCT,
		     CSCCorrelatedLCTDigi& lct1, CSCCorrelatedLCTDigi& lct2, int me);

  std::vector<CSCALCTDigi> alctV;
  std::vector<CSCCLCTDigi> clctV;
  std::vector<CSCALCTDigi> alctV1a;
  std::vector<CSCCLCTDigi> clctV1a;

};
#endif
