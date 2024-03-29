// @(#)root/tmva $Id$
/**********************************************************************************
 * Project   : TMVA - a ROOT-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: TMVAClassification                                                 *
 *                                                                                *
 * This macro provides examples for the training and testing of the               *
 * TMVA classifiers.                                                              *
 *                                                                                *
 * As input data is used a toy-MC sample consisting of four Gaussian-distributed  *
 * and linearly correlated input variables.                                       *
 *                                                                                *
 * The methods to be used can be switched on and off by means of booleans, or     *
 * via the prompt command, for example:                                           *
 *                                                                                *
 *    root -l ./TMVAClassification.C\(\"Fisher,Likelihood\"\)                     *
 *                                                                                *
 * (note that the backslashes are mandatory)                                      *
 * If no method given, a default set of classifiers is used.                      *
 *                                                                                *
 * The output file "TMVA.root" can be analysed with the use of dedicated          *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 * Launch the GUI via the command:                                                *
 *                                                                                *
 *    root -l ./TMVAGui.C                                                         *
 *                                                                                *
 **********************************************************************************/

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <set>
#include <algorithm>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVAGui.C"
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"

using namespace TMVA;

void Fisher() {
	//---------------------------------------------------------------
	// This loads the library
	Tools::Instance();

	std::cerr << std::endl;
	std::cerr << "==> Start TMVAClassification" << std::endl;

	// --------------------------------------------------------------------------------------------------

    // Read training and test data  
    TString data_path = "~/Desktop/";//"/mnt/xrootdb/alister/MVA_studies/samples/"; 
    TFile *input_sig_el = TFile::Open(data_path + "nominal_el/tprime_650_1M.root");
    TFile *input_sig_mu = TFile::Open(data_path + "nominal_mu/tprime_650_1M.root");
    TFile *input_ttbar_el = TFile::Open(data_path + "nominal_el/ttbar.root");
    TFile *input_ttbar_mu = TFile::Open(data_path + "nominal_mu/ttbar.root");
    TFile *input_wjets_el = TFile::Open(data_path + "nominal_el/wjets.root");
    TFile *input_wjets_mu = TFile::Open(data_path + "nominal_mu/wjets.root");
    TFile *input_zjets_el = TFile::Open(data_path + "nominal_el/zjets.root");
    TFile *input_zjets_mu = TFile::Open(data_path + "nominal_mu/zjets.root");
    TFile *input_singletop_el = TFile::Open(data_path + "nominal_el/singletop.root");
    TFile *input_singletop_mu = TFile::Open(data_path + "nominal_mu/singletop.root");
    TFile *input_diboson_el = TFile::Open(data_path + "nominal_el/diboson.root");
    TFile *input_diboson_mu = TFile::Open(data_path + "nominal_mu/diboson.root");

    // --- Register the training and test trees
    TTree *signal_el  = (TTree*)input_sig_el->Get("mini");
    TTree *signal_mu  = (TTree*)input_sig_mu->Get("mini");
    TTree *ttbar_el = (TTree*)input_ttbar_el->Get("mini");
    TTree *ttbar_mu = (TTree*)input_ttbar_mu->Get("mini");
    TTree *wjets_el = (TTree*)input_wjets_el->Get("mini");
    TTree *wjets_mu = (TTree*)input_wjets_mu->Get("mini");
    TTree *zjets_el = (TTree*)input_zjets_el->Get("mini");
    TTree *zjets_mu = (TTree*)input_zjets_mu->Get("mini");
    TTree *singletop_el = (TTree*)input_singletop_el->Get("mini");
    TTree *singletop_mu = (TTree*)input_singletop_mu->Get("mini");
    TTree *diboson_el = (TTree*)input_diboson_el->Get("mini");
    TTree *diboson_mu = (TTree*)input_diboson_mu->Get("mini");
    
	// --- Here the preparation phase begins
    std::vector<std::vector<TString> > variables; //each variable set specified in a 4-tuple.
    std::fstream fin("cleaned-variables-no-jets.txt", std::fstream::in);
    std::vector<TString> inp(4); //var, title, unit, type.
    while (fin >> inp[0] >> inp[1] >> inp[2] >> inp[3]) {
        variables.push_back(inp);
    }
    
    // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
    TString outfileName( "TMVA.root" );
    TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

    // Create the factory object. Later you can choose the methods
    // whose performance you'd like to investigate. The factory is 
    // the only TMVA object you have to interact with
    //
    // The first argument is the base of the name of all the
    // weightfiles in the directory weight/
    //
    // The second argument is the output file for the training results
    // All TMVA output can be suppressed by removing the "!" (not) in
    // front of the "Silent" argument in the option string
    Factory *factory = new Factory( "TMVAClassification", outputFile,
	        "!V:Silent:Color:DrawProgressBar:AnalysisType=Classification" );
    
    for (int i = 0; i < variables.size(); i++) {
        const std::vector<TString>& tup = variables[i];
        if (tup[1] == "analysis_channel") continue;
        std::cerr << "Adding variable: " << tup[1] << endl;
        factory->AddVariable(tup[0], tup[1], tup[2], tup[3][0]);
    }
	std::cerr << "================================================" << std::endl;

    // global event weights per tree (see below for setting event-wise weights)
    Double_t signalWeight     = 1.0;
    Double_t backgroundWeight = 1.0;

    // You can add an arbitrary number of signal or background trees
    factory->AddSignalTree    ( signal_el,     signalWeight     );
    factory->AddSignalTree    ( signal_mu,     signalWeight     );
    factory->AddBackgroundTree( ttbar_el, backgroundWeight );
    factory->AddBackgroundTree( ttbar_mu, backgroundWeight );
    factory->AddBackgroundTree( wjets_el, backgroundWeight );
    factory->AddBackgroundTree( wjets_mu, backgroundWeight );
    factory->AddBackgroundTree( zjets_el, backgroundWeight );
    factory->AddBackgroundTree( zjets_mu, backgroundWeight );
    factory->AddBackgroundTree( singletop_el, backgroundWeight );
    factory->AddBackgroundTree( singletop_mu, backgroundWeight );
    factory->AddBackgroundTree( diboson_el, backgroundWeight );
    factory->AddBackgroundTree( diboson_mu, backgroundWeight );

    // Set individual event weights (the variables must exist in the original TTree)
    factory->SetSignalWeightExpression    ("weight_1btin_70*weight_70/btweight_70");
    factory->SetBackgroundWeightExpression("weight_1btin_70*weight_70/btweight_70");

    // Apply additional cuts on the signal and background samples (can be different)
    TCut mycuts = "weight_70>0 && analysis_channel==1"; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
    TCut mycutb = "weight_70>0 && analysis_channel==1"; // for example: TCut mycutb = "abs(var1)<0.5";

    // Tell the factory how to use the training and testing events
    //
    // If no numbers of events are given, half of the events in the tree are used 
    // for training, and the other half for testing:
    //    factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
    // To also specify the number of testing events, use:
    //    factory->PrepareTrainingAndTestTree( mycut,
    //                                         "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );
    factory->PrepareTrainingAndTestTree( mycuts, mycutb,
	        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );


    factory->BookMethod( TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:VarTransform=Norm:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );
    
    // ---- Book MVA methods
    //
    // Please lookup the various method configuration options in the corresponding cxx files, eg:
    // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
		        
    // ---- Now you can tell the factory to train, test, and evaluate the MVAs

    // Train MVAs using the set of training events
    factory->TrainAllMethods();

    // ---- Evaluate all MVAs using the set of test events
    factory->TestAllMethods();

    // ----- Evaluate and compare performance of all configured MVAs
    factory->EvaluateAllMethods();

    // --------------------------------------------------------------
    
    // Save the output
    outputFile->Close();

    std::cerr << "==> Wrote root file: " << outputFile->GetName() << std::endl;
    std::cerr << "==> TMVAClassification is done!" << std::endl;

	// Launch the GUI for the root macros
	//if (!gROOT->IsBatch()) TMVAGui( outfileName );
	gApplication->Terminate(0);
}

