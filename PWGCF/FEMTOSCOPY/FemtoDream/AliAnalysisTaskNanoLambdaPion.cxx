#include "AliAnalysisTaskNanoLambdaPion.h"
#include "AliFemtoDreamBasePart.h"
#include "AliLog.h"
#include "AliNanoAODTrack.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisManager.h"
#include "AliVEvent.h"
#include "TH1F.h"
#include "TList.h"
#include "AliAODMCParticle.h"

ClassImp(AliAnalysisTaskNanoLambdaPion)
    AliAnalysisTaskNanoLambdaPion::AliAnalysisTaskNanoLambdaPion()
    : AliAnalysisTaskSE(),
      fIsMC(false),
      fUseOMixing(false),
      fPCSettings(NewPC),
      fUseEvtNoLambda(false),
      fExcludedMothers({}),
      fTrigger(AliVEvent::kINT7),
      fQA(nullptr),
      fEvtList(nullptr),
      fLambdaList(nullptr),
      fLambdaMCList(nullptr),
      fAntiLambdaList(nullptr),
      fAntiLambdaMCList(nullptr),
      fPionPlusList(nullptr),
      fPionPlusMCList(nullptr),
      fPionMinusList(nullptr),
      fPionMinusMCList(nullptr),
      fResults(nullptr),
      fResultsQA(nullptr),
      fInputEvent(nullptr),
      fEvent(nullptr),
      fTrack(nullptr),
      fLambda(nullptr),
      fEventCuts(nullptr),
      fPosPionCuts(nullptr),
      fNegPionCuts(nullptr),
      fLambdaCuts(nullptr),
      fAntiLambdaCuts(nullptr),
      fConfig(nullptr),
      fPairCleaner(nullptr),
      fMinvLambdaKStar(false),
      fPionsPlusForMinvV0KStar({}),
      fPionsMinusForMinvV0KStar({}),
      fPartColl(nullptr),
      fSample(nullptr),
      fGTI(nullptr),
      fTrackBufferSize()
{
}

AliAnalysisTaskNanoLambdaPion::AliAnalysisTaskNanoLambdaPion(
    const char *name, bool isMC, PCSettings pcsettings, bool usenolambdaevt)
    : AliAnalysisTaskSE(name),
      fIsMC(isMC),
      fUseOMixing(false),
      fPCSettings(pcsettings),
      fUseEvtNoLambda(usenolambdaevt),
      fExcludedMothers({}),
      fTrigger(AliVEvent::kINT7),
      fQA(nullptr),
      fEvtList(nullptr),
      fLambdaList(nullptr),
      fLambdaMCList(nullptr),
      fAntiLambdaList(nullptr),
      fAntiLambdaMCList(nullptr),
      fPionPlusList(nullptr),
      fPionPlusMCList(nullptr),
      fPionMinusList(nullptr),
      fPionMinusMCList(nullptr),
      fResults(nullptr),
      fResultsQA(nullptr),
      fInputEvent(nullptr),
      fEvent(nullptr),
      fTrack(nullptr),
      fLambda(nullptr),
      fEventCuts(nullptr),
      fPosPionCuts(nullptr),
      fNegPionCuts(nullptr),
      fLambdaCuts(nullptr),
      fAntiLambdaCuts(nullptr),
      fConfig(nullptr),
      fPairCleaner(nullptr),
      fMinvLambdaKStar(false),
      fPionsPlusForMinvV0KStar({}),
      fPionsMinusForMinvV0KStar({}),
      fPartColl(nullptr),
      fSample(nullptr),
      fGTI(nullptr),
      fTrackBufferSize(2000)
{
  DefineOutput(1, TList::Class()); // Output for the Event Class and Pair Cleaner
  DefineOutput(2, TList::Class()); // Output for the Event Cuts
  DefineOutput(3, TList::Class()); // Output for the Lambda Cuts
  DefineOutput(4, TList::Class()); // Output for the AntiLambda Cuts
  DefineOutput(5, TList::Class()); // Output for the PionPlus Cuts
  DefineOutput(6, TList::Class()); // Output for the PionMinus Cuts
  DefineOutput(7, TList::Class()); // Output for the Results
  DefineOutput(8, TList::Class()); // Output for the Results QA

  if (isMC)
  {
    DefineOutput(9, TList::Class());  // Output for the Lambda MC
    DefineOutput(10, TList::Class()); // Output for the AntiLambda MC
    DefineOutput(11, TList::Class()); // Output for the PionPlus MC
    DefineOutput(12, TList::Class()); // Output for the PionMinus MC
  }
}

AliAnalysisTaskNanoLambdaPion::~AliAnalysisTaskNanoLambdaPion() {}

void AliAnalysisTaskNanoLambdaPion::UserCreateOutputObjects()
{

  fEvent = new AliFemtoDreamEvent(true, true, fTrigger);

  fTrack = new AliFemtoDreamTrack();
  fTrack->SetUseMCInfo(
      fPosPionCuts->GetIsMonteCarlo() || fNegPionCuts->GetIsMonteCarlo());

  fLambda = new AliFemtoDreamv0();
  fLambda->SetUseMCInfo(
      fLambdaCuts->GetIsMonteCarlo() || fAntiLambdaCuts->GetIsMonteCarlo());
  fLambda->SetPDGCode(3122);
  fLambda->SetPDGDaughterPos(2212);
  fLambda->GetPosDaughter()->SetUseMCInfo(
      fLambdaCuts->GetIsMonteCarlo() || fAntiLambdaCuts->GetIsMonteCarlo());
  fLambda->SetPDGDaughterNeg(211);
  fLambda->GetNegDaughter()->SetUseMCInfo(
      fLambdaCuts->GetIsMonteCarlo() || fAntiLambdaCuts->GetIsMonteCarlo());

  fGTI = new AliVTrack *[fTrackBufferSize];

  if (!fEventCuts)
  {
    AliFatal("Event Cuts not set!");
  }
  fEventCuts->InitQA();

  fQA = new TList();
  fQA->SetOwner();
  fQA->SetName("QA");
  if (!fConfig->GetMinimalBookingME() && fEvent && fEvent->GetEvtCutList())
  {
    fQA->Add(fEvent->GetEvtCutList());
  }

  if (!fEventCuts->GetMinimalBooking())
  {
    fEvtList = fEventCuts->GetHistList();
  }
  else
  {
    fEvtList = new TList();
    fEvtList->SetName("EventCuts");
    fEvtList->SetOwner();
  }

  if (!fPosPionCuts)
  {
    AliFatal("Track Cuts for Particle One not set!");
  }
  fPosPionCuts->Init();
  fPosPionCuts->SetName("PionPlus");

  if (!fNegPionCuts)
  {
    AliFatal("Track Cuts for Particle One not set!");
  }
  fNegPionCuts->Init();
  fNegPionCuts->SetName("PionMinus");

  if (!fLambdaCuts)
  {
    AliFatal("Track Cuts for Lambda not set!");
  }
  fLambdaCuts->Init();
  fLambdaCuts->SetName("Lambda");

  if (!fAntiLambdaCuts)
  {
    AliFatal("Track Cuts for AntiLambda not set!");
  }
  fAntiLambdaCuts->Init();
  fAntiLambdaCuts->SetName("AntiLambda");

  fPionPlusList = fPosPionCuts->GetQAHists();
  fPionMinusList = fNegPionCuts->GetQAHists();
  fLambdaList = fLambdaCuts->GetQAHists();
  fAntiLambdaList = fAntiLambdaCuts->GetQAHists();

  fPairCleaner =
      new AliFemtoDreamPairCleaner(4, 3, fConfig->GetMinimalBookingME());
  fPartColl =
      new AliFemtoDreamPartCollection(fConfig, fConfig->GetMinimalBookingME());

  fResultsQA = new TList();
  fResultsQA->SetOwner();
  fResultsQA->SetName("ResultsQA");

  if (fConfig->GetUseEventMixing())
  {
    fResults = fPartColl->GetHistList();
    if (!fConfig->GetMinimalBookingME())
    {
      fResultsQA->Add(fPartColl->GetQAList());
      fResultsQA->Add(fPairCleaner->GetHistList());
    }
  }
  else
  {
    fResults = new TList();
    fResults->SetOwner();
    fResults->SetName("Results");
  }

  PostData(1, fQA);
  PostData(2, fEvtList);
  PostData(3, fLambdaList);
  PostData(4, fAntiLambdaList);
  PostData(5, fPionPlusList);
  PostData(6, fPionMinusList);
  PostData(7, fResults);
  PostData(8, fResultsQA);

  if (fLambdaCuts->GetIsMonteCarlo())
  {
    if (!fLambdaCuts->GetMinimalBooking())
    {
      fLambdaMCList = fLambdaCuts->GetMCQAHists();
    }
    else
    {
      fLambdaMCList = new TList();
      fLambdaMCList->SetName("MCv0Cuts");
      fLambdaMCList->SetOwner();
    }
    PostData(9, fLambdaMCList);
  }
  if (fAntiLambdaCuts->GetIsMonteCarlo())
  {
    if (!fAntiLambdaCuts->GetMinimalBooking())
    {
      fAntiLambdaMCList = fAntiLambdaCuts->GetMCQAHists();
    }
    else
    {
      fAntiLambdaMCList = new TList();
      fAntiLambdaMCList->SetName("MCAntiv0Cuts");
      fAntiLambdaMCList->SetOwner();
    }
    PostData(10, fAntiLambdaMCList);
  }

  if (fPosPionCuts->GetIsMonteCarlo())
  {
    if (!fPosPionCuts->GetMinimalBooking())
    {
      fPionPlusMCList = fPosPionCuts->GetMCQAHists();
    }
    else
    {
      fPionPlusMCList = new TList();
      fPionPlusMCList->SetName("MCPosPionCuts");
      fPionPlusMCList->SetOwner();
    }
    PostData(11, fPionPlusMCList);
  }

  if (fNegPionCuts->GetIsMonteCarlo())
  {
    if (!fNegPionCuts->GetMinimalBooking())
    {
      fPionMinusMCList = fNegPionCuts->GetMCQAHists();
    }
    else
    {
      fPionMinusMCList = new TList();
      fPionMinusMCList->SetName("MCNegPionCuts");
      fPionMinusMCList->SetOwner();
    }
    PostData(12, fPionMinusMCList);
  }
}

void AliAnalysisTaskNanoLambdaPion::UserExec(Option_t *)
{

  AliVEvent *Event= fInputEvent;
  
  // PREAMBLE - CHECK EVERYTHING IS THERE
  if (!fInputEvent)
  {
    AliError("No Input event");
    return;
  }

  // EVENT SELECTION
  fEvent->SetEvent(fInputEvent);
  if (!fEventCuts->isSelected(fEvent))
    return;

  ResetGlobalTrackReference();
  for (int iTrack = 0; iTrack < fInputEvent->GetNumberOfTracks(); ++iTrack)
  {
    AliVTrack *track = static_cast<AliVTrack *>(fInputEvent->GetTrack(iTrack));
    if (!track)
    {
      AliFatal("No Standard AOD");
      return;
    }
    StoreGlobalTrackReference(track);
  }

  // Load MC information
  AliMCEvent *fMC = nullptr;
  if (fIsMC) {
    fMC = dynamic_cast<AliAODInputHandler *>(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler())->MCEvent();
  }

  std::vector<AliFemtoDreamBasePart> PionPlus;
  std::vector<AliFemtoDreamBasePart> PionMinus;
  std::vector<AliFemtoDreamBasePart> Lambdas;
  std::vector<AliFemtoDreamBasePart> AntiLambdas;
  std::vector<AliFemtoDreamv0> BeforeCutLambdas;
  std::vector<AliFemtoDreamv0> BeforeCutAntiLambdas;

  AliAODEvent *aodEvt = dynamic_cast<AliAODEvent *>(fInputEvent);
  fLambda->SetGlobalTrackInfo(fGTI, fTrackBufferSize);
  for (int iv0 = 0;
       iv0 < static_cast<TClonesArray *>(aodEvt->GetV0s())->GetEntriesFast();
       ++iv0)
  {
    AliAODv0 *casc = aodEvt->GetV0(iv0);
    fLambda->Setv0(fInputEvent, casc);
    if(fMinvLambdaKStar) {
      cout << "Filling Lambda" << endl;
      // if(fLambda->GetPDGCode()>0) {
        BeforeCutLambdas.push_back(*fLambda);
      // } 
      // if(fLambda->GetPDGCode()<0) {
        cout << "Filling AntiLambda" << endl;
        BeforeCutAntiLambdas.push_back(*fLambda);
      // } 
    }
    cout << "CIAO" << endl;
    cout << fLambdaCuts->isSelected(fLambda) << endl; 
    cout << "CIAO10" << endl;
    if (fLambdaCuts->isSelected(fLambda))
    {
      cout << "CIAO11" << endl;
      if (fMC && fExcludedMothers.size() > 0) {
        // Reject the decay products of some specific particles
        cout << "CIAO12" << endl;
        AliAODMCParticle *mcPart = (AliAODMCParticle *)fMC->GetTrack(fLambda->GetID());
        if (!mcPart) continue;

        cout << "CIAO13" << endl;
        AliAODMCParticle *mom = (AliAODMCParticle *)fMC->GetTrack(mcPart->GetMother());
        if (!mom) continue;

        cout << "CIAO14" << endl;
        int momAbsPdg = std::abs(mom->GetPdgCode());
        if (std::find(fExcludedMothers.begin(), fExcludedMothers.end(), momAbsPdg) != fExcludedMothers.end()) {
          continue;
        cout << "CIAO15" << endl;
        }
      }

      cout << "CIAO16" << endl;
      Lambdas.push_back(*fLambda);
    }
    cout << "CIAO1" << endl;
    if (fAntiLambdaCuts->isSelected(fLambda))
    {
      if (fMC && fExcludedMothers.size() > 0) {
        // Reject the decay products of some specific particles
        AliAODMCParticle *mcPart = (AliAODMCParticle *)fMC->GetTrack(fLambda->GetID());
        if (!mcPart) continue;

        AliAODMCParticle *mom = (AliAODMCParticle *)fMC->GetTrack(mcPart->GetMother());
        if (!mom) continue;

        int momAbsPdg = std::abs(mom->GetPdgCode());
        if (std::find(fExcludedMothers.begin(), fExcludedMothers.end(), momAbsPdg) != fExcludedMothers.end()) {
          continue;
        }
      }

      AntiLambdas.push_back(*fLambda);
    }
    cout << "CIAO2" << endl;
  }

  static float massPion =
      TDatabasePDG::Instance()->GetParticle(fPosPionCuts->GetPDGCode())->Mass();

  fTrack->SetGlobalTrackInfo(fGTI, fTrackBufferSize);

  for (int iTrack = 0; iTrack < fInputEvent->GetNumberOfTracks(); ++iTrack)
  {
    AliVTrack *track = static_cast<AliVTrack *>(fInputEvent->GetTrack(iTrack));

    if (!track)
      continue;
    fTrack->SetTrack(track, fInputEvent);
    if (fPosPionCuts->isSelected(fTrack))
    {
      if (fMC && fExcludedMothers.size() > 0) {
        // Reject the decay products of some specific particles
        AliAODMCParticle *mcPart = (AliAODMCParticle *)fMC->GetTrack(fTrack->GetID());
        if (!mcPart) continue;

        AliAODMCParticle *mom = (AliAODMCParticle *)fMC->GetTrack(mcPart->GetMother());
        if (!mom) continue;

        int momAbsPdg = std::abs(mom->GetPdgCode());
        if (std::find(fExcludedMothers.begin(), fExcludedMothers.end(), momAbsPdg) != fExcludedMothers.end()) {
          continue;
        }
      }

      PionPlus.push_back(*fTrack);
    }
    if (fNegPionCuts->isSelected(fTrack))
    {
      if (fMC && fExcludedMothers.size() > 0) {
        // Reject the decay products of some specific particles
        AliAODMCParticle *mcPart = (AliAODMCParticle *)fMC->GetTrack(fTrack->GetID());
        if (!mcPart) continue;

        AliAODMCParticle *mom = (AliAODMCParticle *)fMC->GetTrack(mcPart->GetMother());
        if (!mom) continue;

        int momAbsPdg = std::abs(mom->GetPdgCode());
        if (std::find(fExcludedMothers.begin(), fExcludedMothers.end(), momAbsPdg) != fExcludedMothers.end()) {
          continue;
        }
      }

      PionMinus.push_back(*fTrack);
    }
  }

  cout << "CIAO3" << endl;
  if (fIsMC)
  {
    for (int iPart = 0; iPart < (fMC->GetNumberOfTracks()); iPart++)
    {
      AliAODMCParticle *mcPart = (AliAODMCParticle *)fMC->GetTrack(iPart);
      if (TMath::Abs(mcPart->Eta()) < 0.8 && mcPart->IsPhysicalPrimary())
      {
        if (mcPart->GetPdgCode() == fPosPionCuts->GetPDGCode())
        {
          fPosPionCuts->FillGenerated(mcPart->Pt());
        }
        else if (mcPart->GetPdgCode() == fNegPionCuts->GetPDGCode())
        {
          fNegPionCuts->FillGenerated(mcPart->Pt());
        }
        else if (mcPart->GetPdgCode() == fLambdaCuts->GetPDGv0())
        {
          fLambdaCuts->FillGenerated(mcPart->Pt());
        }
        else if (mcPart->GetPdgCode() == fAntiLambdaCuts->GetPDGv0())
        {
          fAntiLambdaCuts->FillGenerated(mcPart->Pt());
        }
      }
    }
  }

  fPairCleaner->ResetArray();
  switch(fPCSettings)
  {
      case NoPC : break;
      case OldPC :    fPairCleaner->CleanTrackAndDecay(&PionPlus, &Lambdas, 0, 0);
                      fPairCleaner->CleanTrackAndDecay(&PionMinus, &AntiLambdas, 1, 0);
                      fPairCleaner->CleanTrackAndDecay(&PionPlus, &AntiLambdas, 2, 0);
                      fPairCleaner->CleanTrackAndDecay(&PionMinus, &Lambdas, 3, 0);
                      fPairCleaner->CleanDecay(&Lambdas, 0);
                      fPairCleaner->CleanDecay(&AntiLambdas, 1);
                      fPairCleaner->CleanDecayAndDecay(&Lambdas, &AntiLambdas, 2); 
                      break;
      case NewPC :    fPairCleaner->CleanTrackAndDecay(&PionPlus, &Lambdas, 0, 1);
                      fPairCleaner->CleanTrackAndDecay(&PionMinus, &AntiLambdas, 1, 1);
                      fPairCleaner->CleanTrackAndDecay(&PionPlus, &AntiLambdas, 2, 1);
                      fPairCleaner->CleanTrackAndDecay(&PionMinus, &Lambdas, 3, 1);
                    
                      fPairCleaner->CleanDecay(&Lambdas, 0);
                      fPairCleaner->CleanDecay(&AntiLambdas, 1);
                      fPairCleaner->CleanDecayAndDecay(&Lambdas, &AntiLambdas, 2); 
                      break;
  }

  if(fPCSettings == NewPC && fMinvLambdaKStar) {
    cout << "PairCleaner and InvMass check passed" << endl;
    if(!fPionsPlusForMinvV0KStar.empty()) {
      cout << "Fill histo Lambdas" << endl;
      fLambdaCuts->Fillv0MinvKstarHisto(BeforeCutLambdas, fPionsPlusForMinvV0KStar);
      fAntiLambdaCuts->Fillv0MinvKstarHisto(BeforeCutAntiLambdas, fPionsPlusForMinvV0KStar);
    }
    if(!fPionsMinusForMinvV0KStar.empty()) {
      cout << "Fill histo AntiLambdas" << endl;
      fLambdaCuts->Fillv0MinvKstarHisto(BeforeCutLambdas, fPionsMinusForMinvV0KStar);
      fAntiLambdaCuts->Fillv0MinvKstarHisto(BeforeCutAntiLambdas, fPionsMinusForMinvV0KStar);
    }
    cout << "Updating pions" << endl;
    fPionsPlusForMinvV0KStar = PionPlus;
    fPionsMinusForMinvV0KStar = PionMinus;
  } else if (fPCSettings != NewPC && fMinvLambdaKStar) {
    std::cout << "Invariant mass as a function of kstar not implemented for the selected pair cleaner!" << std::endl;
  } 

  if(fUseEvtNoLambda){
    if(Lambdas.size() == 0 && AntiLambdas.size() == 0){
      return;
    }
  }

  // Create a vector of vector of particles with the pi+, pi-, lambda 
  // and antilambda of the event 
  fPairCleaner->StoreParticle(PionPlus);
  fPairCleaner->StoreParticle(PionMinus);
  fPairCleaner->StoreParticle(Lambdas);
  fPairCleaner->StoreParticle(AntiLambdas);

  if (fPairCleaner->GetCounter() > 0)
  {
    fPartColl->SetEvent(fPairCleaner->GetCleanParticles(),
                        fEvent->GetZVertex(), fEvent->GetRefMult08(),
                        fEvent->GetV0MCentrality());
  }

  PostData(1, fQA);
  PostData(2, fEvtList);
  PostData(3, fLambdaList);
  PostData(4, fAntiLambdaList);
  PostData(5, fPionPlusList);
  PostData(6, fPionMinusList);
  PostData(7, fResults);
  PostData(8, fResultsQA);

  if (fLambdaCuts->GetIsMonteCarlo())
  {
    PostData(9, fLambdaMCList);
  }
  if (fAntiLambdaCuts->GetIsMonteCarlo())
  {
    PostData(10, fAntiLambdaMCList);
  }
  if (fPosPionCuts->GetIsMonteCarlo())
  {
    PostData(11, fPionPlusMCList);
  }
  if (fNegPionCuts->GetIsMonteCarlo())
  {
    PostData(12, fPionMinusMCList);
  }
}

void AliAnalysisTaskNanoLambdaPion::ResetGlobalTrackReference()
{
  for (UShort_t i = 0; i < fTrackBufferSize; i++)
  {
    fGTI[i] = 0;
  }
}

void AliAnalysisTaskNanoLambdaPion::StoreGlobalTrackReference(AliVTrack *track) {
  AliNanoAODTrack *nanoTrack = dynamic_cast<AliNanoAODTrack*>(track);
  const int trackID = track->GetID();
  if (trackID < 0) 
  {
    return;
  }
  if (trackID >= fTrackBufferSize) 
  {
    printf("Warning: track ID too big for buffer: ID: %d, buffer %d\n", trackID,
           fTrackBufferSize);
    return;
  }
  if (fGTI[trackID]) 
  {
    if ((!nanoTrack->GetFilterMap()) && (!track->GetTPCNcls())) 
    {
      return;
    }
    if (dynamic_cast<AliNanoAODTrack*>(fGTI[trackID])->GetFilterMap()
        || fGTI[trackID]->GetTPCNcls()) {
      printf("Warning! global track info already there!");
      printf("         TPCNcls track1 %u track2 %u",
             (fGTI[trackID])->GetTPCNcls(), track->GetTPCNcls());
      printf("         FilterMap track1 %u track2 %u\n",
             dynamic_cast<AliNanoAODTrack*>(fGTI[trackID])->GetFilterMap(),
             nanoTrack->GetFilterMap());
    }
  }
  (fGTI[trackID]) = track;
}
