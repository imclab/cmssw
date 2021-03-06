#ifndef HLTrigger_btau_L3MumuTrackingRegion_H 
#define HLTrigger_btau_L3MumuTrackingRegion_H 

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "RecoTracker/TkTrackingRegions/interface/TrackingRegionProducer.h"
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"
#include "RecoTracker/TkTrackingRegions/interface/RectangularEtaPhiTrackingRegion.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"

class L3MumuTrackingRegion : public TrackingRegionProducer {

public:

  L3MumuTrackingRegion(const edm::ParameterSet& cfg, edm::ConsumesCollector && iC) :  L3MumuTrackingRegion(cfg) {
    if (theVertex) theVertexToken  = iC.consumes<reco::VertexCollection>(theVertexTag);
    if (!(theVertex && useVtxTks)) theInputTrkToken= iC.consumes<reco::TrackCollection>(theInputTrkTag);
  }

  L3MumuTrackingRegion(const edm::ParameterSet& cfg) { 

    edm::ParameterSet regionPSet = cfg.getParameter<edm::ParameterSet>("RegionPSet");

    theVertexTag    = regionPSet.getParameter<edm::InputTag>("vertexSrc");
    theVertex       = (theVertexTag.label().length()>1);
    theInputTrkTag  = regionPSet.getParameter<edm::InputTag>("TrkSrc");

    useVtxTks = regionPSet.getParameter<bool>("UseVtxTks");

    thePtMin              = regionPSet.getParameter<double>("ptMin");
    theOriginRadius       = regionPSet.getParameter<double>("originRadius");
    theOriginHalfLength   = regionPSet.getParameter<double>("originHalfLength");
    theOriginZPos  = regionPSet.getParameter<double>("vertexZDefault");

    theDeltaEta = regionPSet.getParameter<double>("deltaEtaRegion");
    theDeltaPhi =  regionPSet.getParameter<double>("deltaPhiRegion");
    if (regionPSet.exists("searchOpt")){
      m_searchOpt    = regionPSet.getParameter<bool>("searchOpt");
    }
    else{
      m_searchOpt = false;
    }
    m_measurementTracker ="";
    m_howToUseMeasurementTracker=0;
    if (regionPSet.exists("measurementTrackerName")){
      m_measurementTracker = regionPSet.getParameter<std::string>("measurementTrackerName");
      if (regionPSet.exists("howToUseMeasurementTracker")){
	m_howToUseMeasurementTracker = regionPSet.getParameter<double>("howToUseMeasurementTracker");
      }
    }
  }   

  virtual ~L3MumuTrackingRegion(){}

  virtual std::vector<TrackingRegion* > regions(const edm::Event& ev, 
      const edm::EventSetup& es) const {

    std::vector<TrackingRegion* > result;

    // optional constraint for vertex
    // get highest Pt pixel vertex (if existing)
    double deltaZVertex =  theOriginHalfLength;
    double originz = theOriginZPos;
    if (theVertex) {
      edm::Handle<reco::VertexCollection> vertices;
      if (theVertexToken.isUninitialized() || (!ev.getByToken(theVertexToken,vertices))) ev.getByLabel(theVertexTag,vertices);
      const reco::VertexCollection vertCollection = *(vertices.product());
      reco::VertexCollection::const_iterator ci = vertCollection.begin();
      if (vertCollection.size()>0) {
            originz = ci->z();
      } else {
            originz = theOriginZPos;
            deltaZVertex = 15.;
      }
      if (useVtxTks) {

        for(ci=vertCollection.begin();ci!=vertCollection.end();ci++)
          for (reco::Vertex::trackRef_iterator trackIt =  ci->tracks_begin();trackIt !=  ci->tracks_end();trackIt++){
	    reco::TrackRef iTrk =  (*trackIt).castTo<reco::TrackRef>() ;
            GlobalVector dirVector((iTrk)->px(),(iTrk)->py(),(iTrk)->pz());
            result.push_back(
                             new RectangularEtaPhiTrackingRegion( dirVector, GlobalPoint(0,0,float(ci->z())),
                                                                  thePtMin, theOriginRadius, deltaZVertex, theDeltaEta, theDeltaPhi,
								  m_howToUseMeasurementTracker,
								  true,
								  m_measurementTracker,
								  m_searchOpt) );
          }
        return result;
      }
    }

    edm::Handle<reco::TrackCollection> trks;
    if (theInputTrkToken.isUninitialized() || (!ev.getByToken(theInputTrkToken,trks))) ev.getByLabel(theInputTrkTag, trks);

    for(reco::TrackCollection::const_iterator iTrk = trks->begin();iTrk != trks->end();iTrk++) {
      GlobalVector dirVector((iTrk)->px(),(iTrk)->py(),(iTrk)->pz());
      result.push_back( 
          new RectangularEtaPhiTrackingRegion( dirVector, GlobalPoint(0,0,float(originz)), 
					       thePtMin, theOriginRadius, deltaZVertex, theDeltaEta, theDeltaPhi,
					       m_howToUseMeasurementTracker,
					       true,
					       m_measurementTracker,
					       m_searchOpt) );
    }
    return result;
  }

private:

  edm::InputTag                            theVertexTag;
  bool                                     theVertex;
  edm::EDGetTokenT<reco::VertexCollection> theVertexToken;
  edm::InputTag                            theInputTrkTag;
  edm::EDGetTokenT<reco::TrackCollection>  theInputTrkToken;

  bool useVtxTks;

  double thePtMin; 
  double theOriginRadius; 
  double theOriginHalfLength; 
  double theOriginZPos;

  double theDeltaEta; 
  double theDeltaPhi;
  std::string m_measurementTracker;
  double m_howToUseMeasurementTracker;
  bool m_searchOpt;
};

#endif 

